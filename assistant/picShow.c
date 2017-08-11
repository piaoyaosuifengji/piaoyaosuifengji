#include <cairo.h>
#include <math.h>
#include <hiredis/hiredis.h>
#include <alsa/asoundlib.h>  
#include <alsa/pcm.h>
#include  "fftw3.h"  
#include "../src/common/type.h"
#include "../src/common/sounds.h"
#include "fileOperating.h"

// #include <alsa/asoundlib.h>  



#define StartXorigin  50

#define MaxCoordinateNum  100

#define SpaceBetweenSoundValue  5 //坐标System一个单位长度用几个像素来表示,need to test  with  wav HZ
#define YPositiveAxis_  200//y正方向轴显示几个像素de 长度   
#define spaceBetweenCS_  15//y正方向轴显示几个像素de 长度   



#define STRETCH  100
#define INTERPOLATION  101  //插值

static nero_8int  file_path_getcwd[FILEPATH_MAX]="/tmp";/*保存当前目录*/

int playWave16kChannel1(nero_8int * waveData,nero_us32int dataNum ) ;

struct PicLayoutParameter
{
    nero_us32int width;
    nero_us32int length;
    POS          origins[MaxCoordinateNum];// Coordinate System  origin
    nero_us32int currentCoordinateNum;
    nero_us32int CoordinateUnitLen;         //坐标System一个单位长度用几个像素来表示
    nero_us32int XPositiveAxis;               // x正方向轴显示几个像素de 长度
    nero_us32int YPositiveAxis;               // y正方向轴显示几个像素de 长度   
    nero_us32int spaceBetweenCS;               // 坐标系y方向的间隔
};
//  csNum   ----  坐标系的个数
void initPicLayoutParameter(struct PicLayoutParameter *  layoutPara ,nero_us32int width,nero_us32int length , nero_us32int csNum)
{

    nero_us32int i;

    layoutPara->width = width;
    layoutPara->length=length;
    layoutPara->currentCoordinateNum =0;
    layoutPara->CoordinateUnitLen = SpaceBetweenSoundValue ;
    layoutPara->XPositiveAxis =     2000 ;
    layoutPara->YPositiveAxis =     YPositiveAxis_ ;
    layoutPara->spaceBetweenCS =     spaceBetweenCS_ ;


    if(csNum > MaxCoordinateNum  ||   length <  500 ||  width < (YPositiveAxis_ *2 +  spaceBetweenCS_) )
    {
        return;
    }
    //图像的最上面是第一个cs
    for(i=0;i<csNum;i++)
    {
         (layoutPara->origins)[i].x =  StartXorigin;
         (layoutPara->origins)[i].y =  (i+1) * layoutPara->spaceBetweenCS   +  layoutPara->YPositiveAxis * (2 * i+1);

    }

}

//set Coordinate System in cairo_surface

//layoutPara 中可以放入多个坐标系，pos表示，此次的数据写入第pos个坐标系中
//pos start from 0
void drawCoordinateInSurface(cairo_t *cr , struct PicLayoutParameter *  layoutPara, nero_8int *  dbListName ,nero_us32int pos,redisContext* c)
{

    nero_s32int start,stop,width,length,i,x,y,j,tmpi,tmpV,sampleRate,XPositiveAxis,YPositiveAxis,bytesPerSecond,numPerSecond;
    nero_s32int originX,originY,maxWaveVal;//原点
    redisReply* r_tmp1;
    double  coefficient;       // wave data转化为y轴系数   
    nero_s32int  showDataPerSed ;//在图像中显示每秒data中多少个数据

    short int yy;
    nero_us32int showOneDataInwaves,dataHasBeenShow;// 每过多少帧采集一个数据在坐标中显示
    if(cr == NULL || layoutPara == NULL ||   dbListName == NULL || pos < 0)
    {

        printf("drawCoordinateInSurface: para error \n" );
        return ;

    }

    if(pos > layoutPara->currentCoordinateNum)
    {
        printf("drawCoordinateInSurface: no enough  CoordinateNum  pos=%d\n",pos );
        return ;    
    }
    originX  =  (layoutPara-> origins)[pos ].x;
    originY  =  (layoutPara-> origins)[pos ].y;
    width  =  (layoutPara-> width) ;
    length  =  (layoutPara-> length) ;
    XPositiveAxis = (layoutPara-> XPositiveAxis) ;
    YPositiveAxis =(layoutPara-> YPositiveAxis) ;
    printf("dbListName  = %s \n",dbListName  );
    printf("originX  = %d, originY  =%d,  width  =%d,  length  =  %d\n",originX,originY,width,length );
    start = 0;
    stop =  -1;
    // num = stop - start +1;
    r_tmp1= (redisReply*)redisCommand(c,"LRANGE %s %d %d",dbListName,start,stop);
    // r_tmp1 = redisCommand(c, "HGETALL %s",neroAddressTable);
    if (!(r_tmp1->type == REDIS_REPLY_ARRAY &&  (r_tmp1->elements ) > 0    ))
    {
        printf("Failed to LRANGE  elements =%d\n" ,r_tmp1->elements );
        freeReplyObject(r_tmp1);
    }       
    else
    {

        // 设置白色背景
        // cairo_set_source_rgba (cr, 1, 0.0, 0.0, 1.0);
        cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);//make the text color
        cairo_move_to (cr, 0,0);
        cairo_rel_line_to (cr, length, 0);
        cairo_rel_line_to (cr, 0, width);
        cairo_rel_line_to (cr, -length, 0);
        cairo_rel_line_to (cr, 0, -width);
        // cairo_stroke (cr);
        cairo_fill (cr);

        //x Y zhou 
        cairo_set_source_rgb (cr,  0,  0,  0);//black
        cairo_move_to (cr, originX,originY);
        cairo_rel_line_to (cr,layoutPara->XPositiveAxis + originX, 0);
        cairo_move_to (cr, originX,originY);
        cairo_rel_line_to (cr , 0,-1 * (layoutPara->YPositiveAxis)); 
        cairo_move_to (cr, originX,originY);
        cairo_rel_line_to (cr , 0, (layoutPara->YPositiveAxis)); 
        cairo_stroke (cr);

      
        cairo_set_line_width (cr, 4.0);
        cairo_set_source_rgba (cr,  0,  0,1.0, 1.0);//blue
        //x  zhou 刻度值  200/kedu
        for(i=0 ;i <10;i++)
        {
            cairo_move_to (cr, originX + ((i+1) *  (layoutPara->XPositiveAxis /10)  ),originY - 10);
            cairo_rel_line_to (cr , 0, 20);            
            cairo_stroke (cr);

        }
        cairo_set_line_width (cr, 1.0);
        cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 1.0);//red
  
        bytesPerSecond =   ( int) strtol(r_tmp1->element[ r_tmp1->elements-2  ]->str ,NULL,16);
        sampleRate      =  ( int) strtol(r_tmp1->element[ r_tmp1->elements-1  ]->str ,NULL,16);
        //2的16次方是65536，一半就是32768，但是实际上，一般不会真的取到极值，所以这里
        // 只显示0--5000的情况
        // coefficient = ( (layoutPara->YPositiveAxis )) / (double) 5000 ;
        coefficient = 1 ;


        //设置  just 总共输出10秒内的wav文件的单通道数据

        //计算在图中一秒的间隔内显示几个数据的图形,    40  /  16000   too little
        // if layoutPara->length = 2000,so showDataPerSed =  40 /secd
        showDataPerSed =   layoutPara->XPositiveAxis  /10/layoutPara->CoordinateUnitLen ; //指的是在一定的pic长度下适合显示该数量的数据个数
                            //          2000        10 second           5
                            // Bitys  2channels ,16 bit pcm
        // numPerSecond =  bytesPerSecond /8/2/2;           //该wav文件实际每秒的数据个数
        numPerSecond =  sampleRate;           //该wav文件实际每秒的数据(sample)个数
        //也即是说，你需要zai 每  numPerSecond 个数据中采集  showDataPerSed个数据来显示
        showOneDataInwaves = numPerSecond /  showDataPerSed ;

        dataHasBeenShow = 0;
        maxWaveVal = 0;
        printf(" elements =%d,bytesPerSecond =%d,showOneDataInwaves =%d\n",r_tmp1->elements ,bytesPerSecond,showOneDataInwaves);
        for( i=0,  j=r_tmp1->elements-3,tmpi=1 ;j >= 0 ; j--,tmpi++)
        {
            // printf("%s\n" ,r_tmp1->element[j]->str );
            // sscanf(r_tmp1->element[j]->str,"%d",&yy);
            if( dataHasBeenShow  >=  (showDataPerSed * 10))
            {
                break;
            }
            if(tmpi == showOneDataInwaves )
            {
                // i++;
                // y = ( int) strtol(r_tmp1->element[j]->str ,NULL,16);
                // // if(i < 15)
                //     // printf("  %d \n"  ,y );
                // x = layoutPara->CoordinateUnitLen + originX+ i * (layoutPara->CoordinateUnitLen);
                // y = originY-   (coefficient * (double)y) ;
                // printf("  %d %d \n" ,x ,y );
                // cairo_arc (cr, x ,y, 1.0, 0, 2*M_PI);// cr,x,y ,r,...
                // cairo_fill (cr);
                // tmpi =0;
                i++;dataHasBeenShow++;
                tmpV = ( int) strtol(r_tmp1->element[j]->str ,NULL,16);
                if(maxWaveVal < tmpV)
                {
                    maxWaveVal = tmpV;
                    // printf("maxWaveVal:  %d \n"  ,tmpV );
                }
                x = layoutPara->CoordinateUnitLen + originX+ i * (layoutPara->CoordinateUnitLen);
                // printf("  %d %d \n" ,x ,y );
                cairo_move_to (cr, x,y);
                if( tmpV>0 )
                {
                    y = originY-   (coefficient * (double)tmpV) ;
                    cairo_move_to (cr, x,originY);
                    cairo_rel_line_to (cr , 0, -(coefficient * (double)tmpV)); 
                }
                else
                {
                     y = originY+  (coefficient * (double)tmpV) ;
                     cairo_move_to (cr, x,originY);
                     cairo_rel_line_to (cr , 0, -(coefficient * (double)tmpV) ); 
                }
                cairo_stroke (cr);
                tmpi =0;
            }

        }
        freeReplyObject(r_tmp1);
    }    
    printf("dataHasBeenShow =%d,showDataPerSed=%d,coefficient=%f\n",dataHasBeenShow,showDataPerSed,coefficient);
                    printf("maxWaveVal:  %d \n"  ,maxWaveVal );

    //画个小圆圈(dian)
    // cairo_set_source_rgba (cr, 1, 0.0, 0.0, 1.0);
    // cairo_set_line_width (cr, 1.0);
    // cairo_arc (cr, 20, 20, 1.0, 0, 2*M_PI);// cr,x,y ,r,...
    // cairo_fill (cr);
}


// "/data/sound/buzui.wav";
int create_pic_forWave( nero_8int * fileName3)
{
    // nero_8int * fileName3="/data/sound/buzui.wav";
    nero_s32int start,stop,num,i,j;
    redisContext* c;
    redisReply* r_tmp1;
    redisReply* r;


    struct PicLayoutParameter   layoutPara;
    nero_us32int csNum;
    nero_us32int width, length;


    c = redisConnect("127.0.0.1", 6379);
    if (c->err) 
    {
        printf("Failed to redisConnect in picshow \n");
        redisFree(c);
        return;
    }
    const char* command1 = "select 0";
    r = (redisReply*)redisCommand(c,command1);
    if (NULL == r) 
    {
        redisFree(c);
        return;
    }
    if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0)) 
    {
        printf("Failed to execute command[%s].\n",command1);
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    // start = -10;
    // stop =  -1;
    // num = stop - start +1;
    // r_tmp1= (redisReply*)redisCommand(c,"LRANGE %s %d %d",fileName3,start,stop);
    // if (!(r_tmp1->type == REDIS_REPLY_ARRAY /*&&  (r_tmp1->elements ) == (neroNumbers*2)  */ ))
    // {
    //     printf("Failed to LRANGE  elements =%d\n" ,r_tmp1->elements );
    //     freeReplyObject(r_tmp1);
    // }       
    // else
    // {
    //     printf(" elements =%d\n" ,r_tmp1->elements );
    //     for( i=start, j=0;i <= stop ;i++,j++)
    //     {
    //         printf("%s\n" ,r_tmp1->element[j]->str );
    //     }
    //     freeReplyObject(r_tmp1);
    // }
    

    csNum =  1; //  
    width =   csNum * (spaceBetweenCS_    + YPositiveAxis_  * 2 + 50) ;   
    /*
    

    */
    length =  2000+ StartXorigin ;// just show 20 secends  in pic 
    //cairo_surface_t is the abstract type representing all different drawing targets that cairo can render to. 
    cairo_surface_t *surface =cairo_image_surface_create (CAIRO_FORMAT_ARGB32, length, width);
    // cairo_t is the main object used when drawing with cairo. 
    cairo_t *cr =cairo_create (surface);
    //选择字体类型或尺寸 [text — Rendering text and glyphs]
    cairo_select_font_face (cr, "serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size (cr, 42.0);

    //Sets the source pattern within cr to an opaque不透明 color.
    // cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);//make the text color
    //Begin a new sub-path. After this call the current point will be (x , y ).
    // cairo_move_to (cr, 100.0, 50.0);
    //A drawing operator that generates the shape from a string of UTF-8 characters, r
    // endered according to the current font_face, font_size (font_matrix), and font_options.
    // cairo_show_text (cr, "Hello1");

    // cairo_move_to (cr, 100.0, 100.0);
    // cairo_show_text (cr, "Hello2");



    initPicLayoutParameter(&layoutPara , width, length ,   csNum);
    drawCoordinateInSurface( cr , &layoutPara, fileName3 ,0,  c);

    cairo_destroy (cr);
    cairo_surface_write_to_png (surface, "hello.png");
    cairo_surface_destroy (surface);

    
    redisFree(c);   
    return 0;
}


void mixerTest()
{
/*
long alsa_min_vol, alsa_max_vol;
long int a, b;

snd_mixer_t * mixer;
snd_mixer_elem_t *pcm_element;
long ll, lr;


//初始化
snd_mixer_open(&mixer, 0);
snd_mixer_attach(mixer, "default");
snd_mixer_selem_register(mixer, NULL, NULL);
snd_mixer_load(mixer);
//找到Pcm对应的element,方法比较笨拙
pcm_element = snd_mixer_first_elem(mixer);
pcm_element = snd_mixer_elem_next(pcm_element);
pcm_element = snd_mixer_elem_next(pcm_element);

 
///处理alsa1.0之前的bug，之后的可略去该部分代码
snd_mixer_selem_get_playback_volume(pcm_element, SND_MIXER_SCHN_FRONT_LEFT, &a);
snd_mixer_selem_get_playback_volume(pcm_element, SND_MIXER_SCHN_FRONT_RIGHT, &b);
snd_mixer_selem_get_playback_volume_range(pcm_element, &alsa_min_vol, &alsa_max_vol);
///设定音量范围
snd_mixer_selem_set_playback_volume_range(pcm_element, 0, 100);



//读音量值-
//处理事件
snd_mixer_handle_events(mixer);
//左声道
snd_mixer_selem_get_playback_volume(pcm_element, SND_MIXER_SCHN_FRONT_LEFT, &ll);
//右声道
snd_mixer_selem_get_playback_volume(pcm_element, SND_MIXER_SCHN_FRONT_RIGHT, &lr);


// //写入音量-----------------------------------------------------
// //左音量
// snd_mixer_selem_set_playback_volume(pcm_element, SND_MIXER_SCHN_FRONT_LEFT, leftright);
// //右音量
// snd_mixer_selem_set_playback_volume(pcm_element, SND_MIXER_SCHN_FRONT_RIGHT, leftright);
*/

    int unmute, chn;
    int al, ar;
    snd_mixer_t *mixer;
    snd_mixer_elem_t *master_element;
     
    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, "default");
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);  
    /* 取得第一個 element，也就是 Master */
    master_element = snd_mixer_first_elem(mixer);  
    /* 設定音量的範圍 0 ~ 100 */  
    snd_mixer_selem_set_playback_volume_range(master_element, 0, 100);  
    /* 取得 Master 是否靜音 */  
    snd_mixer_selem_get_playback_switch(master_element, 0, &unmute);  
    if (unmute)   
    {
      printf("Master is Unmute.\n");  

          /* 將 Master 切換為靜音 */  
        for (chn=0;chn<=SND_MIXER_SCHN_LAST;chn++) 
        {      
          snd_mixer_selem_set_playback_switch(master_element, chn, 0);  
        }        
    }   
    else     
    {
         printf("Master is Mute.\n"); 

         /* 將 Master 切換為非靜音 */  
        for (chn=0;chn<=SND_MIXER_SCHN_LAST;chn++) 
        {      
          snd_mixer_selem_set_playback_switch(master_element, chn, 1);  
        }       
    } 

    //   /* 取得左右聲道的音量 */  
    // snd_mixer_selem_get_playback_volume(master_element, SND_MIXER_SCHN_FRONT_LEFT, &al);  
    // snd_mixer_selem_get_playback_volume(master_element, SND_MIXER_SCHN_FRONT_RIGHT, &ar);  
    // /* 兩聲道相加除以二求平均音量 */  
    // printf("Master volume is %d\n", (al + ar) >> 1);  /* 設定 Master 音量 */  
    // snd_mixer_selem_set_playback_volume(master_element, SND_MIXER_SCHN_FRONT_LEFT, 99);  
    // snd_mixer_selem_set_playback_volume(master_element, SND_MIXER_SCHN_FRONT_RIGHT, 99);  





}
void playWaveFromDB(nero_8int * waveName)
{
    nero_s32int start,stop,num,i,j,bytesPerSecond,tmpi,tmpV;
    redisContext* c;
    redisReply* r_tmp1;
    redisReply* r;
    nero_8int   dbListName[1024]  ;

    nero_s16int * buff;
    c = redisConnect("127.0.0.1", 6379);
    if (c->err) 
    {
        printf("Failed to redisConnect in picshow \n");
        redisFree(c);
        return;
    }
    const char* command1 = "select 0";
    r = (redisReply*)redisCommand(c,command1);
    if (NULL == r) 
    {
        redisFree(c);
        return;
    }
    if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0)) 
    {
        printf("Failed to execute command[%s].\n",command1);
        freeReplyObject(r);
        redisFree(c);
        return;
    }

    start = 0;
    stop =  -1;
    sprintf(dbListName,"/data/sound/%s",waveName);
    r_tmp1= (redisReply*)redisCommand(c,"LRANGE %s %d %d",dbListName,start,stop);
    // printf("dbListName:[%s].\n",dbListName);
    // return;
    if (!(r_tmp1->type == REDIS_REPLY_ARRAY &&  (r_tmp1->elements ) > 0    ))
    {
        printf("Failed to LRANGE  elements =%d\n" ,r_tmp1->elements );
        freeReplyObject(r_tmp1);
    }  
    bytesPerSecond =   ( int) strtol(r_tmp1->element[ r_tmp1->elements-2  ]->str ,NULL,16);
    buff =   (nero_s16int  *) malloc( sizeof(nero_s16int) *  r_tmp1->elements );

    #ifdef Nero_DeBuging08_09_17
    for( i=0,  j=r_tmp1->elements-3 ;j >= 0 ; j--,i++)
    {
        // printf("%s\n" ,r_tmp1->element[j]->str );
        // sscanf(r_tmp1->element[j]->str,"%d",&yy);
 
        tmpV = ( int) strtol(r_tmp1->element[j]->str ,NULL,16);
        //     printf("  %d \n"  ,tmpV );

        buff[i] =(nero_s16int ) tmpV;
    }
    #endif
    playWave16kChannel1(buff,i *2 );


    freeReplyObject(r_tmp1);
    free(buff);
}
int playWavFile( )  
{  
    nero_8int * fileName3="/home/jty/nero/nero/src/data/sound/buzui.wav";

    int rc;  
    int size;  
    snd_pcm_t *handle;  
    snd_pcm_hw_params_t *params;  
    unsigned int val;  
    int dir;  
    snd_pcm_uframes_t frames;  
    char *buffer;  
    FILE *fp;  
      

    // mixerTest();
    // return 0;


    fp = fopen(fileName3, "rb");  
    if (!fp) {  
        fprintf(stderr, "can't open sound file\n");
        exit(1);  
    }  
  
    /* Open PCM device for playback. */  
    rc = snd_pcm_open(&handle, "default",SND_PCM_STREAM_PLAYBACK, 0);  
    if (rc < 0)
     {  
        fprintf(stderr,"unable to open pcm device: %s\n", snd_strerror(rc));  
        exit(1);  
    }  
  
    /* Allocate a hardware parameters object. */  
    snd_pcm_hw_params_alloca( &params);  
  
    /* Fill it in with default values. */  
    snd_pcm_hw_params_any(handle, params);  
  
    /* Set the desired hardware parameters. */  
  
    /* Interleaved mode */  
    snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);  
  
    /* Signed 16-bit little-endian format *****bitsPerSample:16 */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
  
    /* Two channels (stereo) */  
    snd_pcm_hw_params_set_channels(handle, params, 2);  
  
    /* 44100 bits/second sampling rate (CD quality)     sampleRate:44100 */
    val = 44100;  
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
  
    /* Set period size to 32 frames. 一次送人的帧*/
    frames = 32;  
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
  
    /* Write the parameters to the driver */  
    rc = snd_pcm_hw_params(handle, params);  
    if (rc < 0) {  
        fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);  
    }  
  
    /* Use a buffer large enough to hold one period */  
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4; /* 2 bytes/sample, 2 channels */  
    buffer = (char *)malloc(size);  
  
    fseek(fp, 44, SEEK_SET);  
    while (1) {  
        rc = fread(buffer, 1, size, fp);  
        if (rc == 0) 
        {  
            fprintf(stderr, "end of file on input\n");  
            break;  
        } else if (rc != size)
        {  
            fprintf(stderr, "short read: read %d bytes\n", rc);
        }  
        rc = snd_pcm_writei(handle, buffer, frames);  
        if (rc == -EPIPE)
        {  
            /* EPIPE means underrun */  
            fprintf(stderr, "underrun occurred\n");  
            snd_pcm_prepare(handle);  
        } 
        else if (rc < 0) 
        {  
            fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
        } 
        else if (rc != (int)frames) 
        {  
            fprintf(stderr, "short write, write %d frames\n", rc);
        }  
    }  
  
    snd_pcm_drain(handle);  
    snd_pcm_close(handle);  
    fclose(fp);  
    free(buffer);  
  
    return 0;  
}  

//  hz = 16000
/*
compressionCode:1 
numChannels:1 
sampleRate:16000 
bytesPerSecond:32000 
blockAlign:2 
bitsPerSample:16

dataNum 总字节数
*/
int playWave16kChannel1(nero_8int * waveData,nero_us32int dataNum )  
{  
    // nero_8int * fileName3="/home/jty/nero/nero/src/data/sound/buzui.wav";

    int rc,p;  
    int size;  
    snd_pcm_t *handle;  
    snd_pcm_hw_params_t *params;  
    unsigned int val;  
    int dir;  
    snd_pcm_uframes_t frames;  
    char *buffer;  
    FILE *fp;  
    int remaining = dataNum;

    // mixerTest();
    // return 0;

  
    /* Open PCM device for playback. */  
    rc = snd_pcm_open(&handle, "default",SND_PCM_STREAM_PLAYBACK, 0);  
    if (rc < 0)
     {  
        fprintf(stderr,"unable to open pcm device: %s\n", snd_strerror(rc));  
        exit(1);  
    }  
  
    /* Allocate a hardware parameters object. */  
    snd_pcm_hw_params_alloca( &params);  
  
    /* Fill it in with default values. */  
    snd_pcm_hw_params_any(handle, params);  
  
    /* Set the desired hardware parameters. */  
  
    /* Interleaved mode */  
    snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);  
  
    /* Signed 16-bit little-endian format *****bitsPerSample:16 */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
  
    /* Two channels (stereo) */  
    snd_pcm_hw_params_set_channels(handle, params, 1);  
  
    /* 44100 bits/second sampling rate (CD quality)     sampleRate:44100 */
    val = 16000;  
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
  
    /* Set period size to 32 frames. 一次送人的帧*/
    frames = 32;  
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
  
    /* Write the parameters to the driver */  
    rc = snd_pcm_hw_params(handle, params);  
    if (rc < 0) {  
        printf("unable to set hw parameters: %s\n", snd_strerror(rc));
        exit(1);  
    }  
  
    /* Use a buffer large enough to hold one period */  
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 2; /* 2 bytes/sample, 2 channels */  
    buffer = (char *)malloc(size);  
    // printf("buffer=%x\n",buffer );
    p =0;
    while (remaining > 0) 
    {  
        // printf("remaining=%d  \n",remaining );
        if(remaining >= size)
        {
            rc = size;
             memcpy(buffer, & (waveData[p]), rc);
            remaining -=size;
            p += rc;
        }
         else
        {
            rc = remaining;
            remaining =0;
             // memcpy(buffer, waveData[p], rc);
             memcpy(buffer, & (waveData[p]), rc);
             p += rc;
        }
        if (rc == 0) 
        {  
            fprintf(stderr, "end of file on input\n");  
            break;  
        } else if (rc != size)
        {  
            fprintf(stderr, "short read: read %d bytes\n", rc);
        }  
        rc = snd_pcm_writei(handle, buffer, frames);  
        if (rc == -EPIPE)
        {  
            /* EPIPE means underrun */  
            fprintf(stderr, "underrun occurred\n");  
            snd_pcm_prepare(handle);  
        } 
        else if (rc < 0) 
        {  
            fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
        } 
        else if (rc != (int)frames) 
        {  
            fprintf(stderr, "short write, write %d frames\n", rc);
        }  
    }  
  
    snd_pcm_drain(handle);  
    snd_pcm_close(handle);  
    // fclose(fp);  
    free(buffer);  
  
    return 0;  
}  
int IS_LITTLE_ENDIAN(void)   
{  
    int __dummy = 1;  
    return ( *( (unsigned char*)(&(__dummy) ) ) );  
}  
  // LITTLE_ENDIAN小字节序、低字节序）,即低位字节排放在内存的低地址端，高位字节排放在内存的高地址端
unsigned int readHeader(void *dst, signed int size, signed int nmemb, FILE *fp)   
{  
    unsigned int n, s0, s1, err;  
    unsigned char tmp, *ptr;  
      
    if ((err = fread(dst, size, nmemb, fp)) != nmemb)   
    {  
        return err;  
    }  
    if (!IS_LITTLE_ENDIAN() && size > 1)   
    {  
        //printf("big-endian \n");  
        ptr = (unsigned char*)dst;  
        for (n=0; n<nmemb; n++)   
        {  
            for (s0=0, s1=size-1; s0 < s1; s0++, s1--)   
            {  
                tmp = ptr[s0];  
                ptr[s0] = ptr[s1];  
                ptr[s1] = tmp;  
            }  
            ptr += size;  
        }  
    }  
    else  
    {  
        //printf("little-endian \n");  
    }  
      
    return err;  
}  
void modifyWaveData(nero_8int *  fileName3 ,int operatorKind,float coefficient)
{

    int rc,addSize;  
    int dataSize,i,j;  
    snd_pcm_t *handle;  
    unsigned int val;  
    int dir;  
    char *buffer;  
    char *buffer2;  
    nero_s16int  dataTmp;
    nero_s16int  * dataP;
    FILE *fp;  
      
    fp = fopen(fileName3, "rb+");  
    if (!fp) {  
        fprintf(stderr, "can't open sound file:%s\n",fileName3);
        exit(1);  
    }  

    fseek(fp, 40, SEEK_SET);  
    readHeader(&dataSize, 4, 1,  fp)   ;

    
    // printf("dataSize=%d  \n",dataSize);
    // buffer = (char *)malloc( dataSize * sizeof(char )) ;
    // fread(buffer, 1, dataSize, fp);

    // val =   (unsigned int )*((unsigned int *) &dataSize);
    // printf("1=%x\n",val );

    //is time to Vary the value of wave data
    buffer2 = NULL;
    switch(operatorKind)
    {
        // the data  is   little-endian format , 16bits  SND_PCM_FORMAT_S16_LE
        case STRETCH:
            printf("dataSize=%d  \n",dataSize);
            buffer = (char *)malloc( dataSize * sizeof(char )) ;
            fread(buffer, 1, dataSize, fp);      
            //做伸缩变化
            // for(i=0;i< (15) ; i++)
            for(i=0;i< (dataSize /2 ) ; i++)
            {
                dataTmp =        *((nero_s16int *)(&buffer[2*i]));
                dataP = (nero_s16int *) (&buffer[2*i]);  

                // printf("i=%d,dataTmp=%d\n",i,dataTmp );
                *dataP = ((float )dataTmp) * coefficient;
            }
            fseek(fp, 44, SEEK_SET);  
            for(i=0;i< (dataSize /2 ) ; i++)
            {
                dataTmp =        *((nero_s16int *)(&buffer[2*i]));
                dataP = (nero_s16int *) (&buffer[2*i]);  

                // printf("i=%d,dataTmp=%d\n",i,dataTmp );
                fwrite(&dataTmp, sizeof(nero_s16int), 1, fp);
            }

            break;

        case INTERPOLATION:
            printf("dataSize=%d  \n",dataSize);
            buffer = (char *)malloc( dataSize * sizeof(char )) ;
            fread(buffer, 1, dataSize, fp);
            buffer2 = (char *)malloc( 2*dataSize * sizeof(char )) ;
            addSize =0;
            //每隔俩个data插入一个新值
            for(i=0,j =1;i< (dataSize   ) ; i++,j++)
            {
                // for(j=1;j<=  2  ; j++)
                {
                    if(j ==1)
                    {
                        buffer2[i+addSize] = buffer[i];
                    }
                    else
                    {
                        j =0;
                        buffer2[i+addSize] = buffer[i];
                        buffer2[i+addSize+1] = coefficient;
                        addSize++;
                    }
                }
            }
            dataSize = dataSize + addSize ;
            free(buffer);
            buffer =buffer2;
            break;
        defualt:
            // just play the original wave file
            printf("dataSize=%d  \n",dataSize);
            buffer = (char *)malloc( dataSize * sizeof(char )) ;
            fread(buffer, 1, dataSize, fp);       
            break ;
    }
    fflush(fp);
    fclose(fp);  
    free(buffer);
}
void wavePlayInit(nero_8int *  fileName3 ,int operatorKind,float coefficient)
{
    // nero_8int * fileName3="/home/jty/nero/nero/src/data/sound/001101_L.wav";001102_L.wav
    // nero_8int * fileName3="/home/jty/nero/nero/src/data/sound/001102_L.wav";

    int rc,addSize;  
    int dataSize,i,j;  
    snd_pcm_t *handle;  
    unsigned int val;  
    int dir;  
    char *buffer;  
    char *buffer2;  
    nero_s16int  dataTmp;
    nero_s16int  * dataP;
    FILE *fp;  
      
    fp = fopen(fileName3, "rb");  
    if (!fp) {  
        fprintf(stderr, "can't open sound file:%s\n",fileName3);
        exit(1);  
    }  

    fseek(fp, 40, SEEK_SET);  
    readHeader(&dataSize, 4, 1,  fp)   ;

    
    // printf("dataSize=%d  \n",dataSize);
    // buffer = (char *)malloc( dataSize * sizeof(char )) ;
    // fread(buffer, 1, dataSize, fp);

    // val =   (unsigned int )*((unsigned int *) &dataSize);
    // printf("1=%x\n",val );

    //is time to Vary the value of wave data
    buffer2 = NULL;
    switch(operatorKind)
    {
        // the data  is   little-endian format , 16bits  SND_PCM_FORMAT_S16_LE
        case STRETCH:
            printf("dataSize=%d  \n",dataSize);
            buffer = (char *)malloc( dataSize * sizeof(char )) ;
            fread(buffer, 1, dataSize, fp);      
            //做伸缩变化
            // for(i=0;i< (15) ; i++)
            for(i=0;i< (dataSize /2 ) ; i++)
            {
                dataTmp =        *((nero_s16int *)(&buffer[2*i]));
                dataP = (nero_s16int *) (&buffer[2*i]);  

                // printf("i=%d,dataTmp=%d\n",i,dataTmp );
                *dataP = ((float )dataTmp) * coefficient;

            }
            break;

        case INTERPOLATION:
            printf("dataSize=%d  \n",dataSize);
            buffer = (char *)malloc( dataSize * sizeof(char )) ;
            fread(buffer, 1, dataSize, fp);
            buffer2 = (char *)malloc( 2*dataSize * sizeof(char )) ;
            addSize =0;
            //每隔俩个data插入一个新值
            for(i=0,j =1;i< (dataSize   ) ; i++,j++)
            {
                // for(j=1;j<=  2  ; j++)
                {
                    if(j ==1)
                    {
                        buffer2[i+addSize] = buffer[i];
                    }
                    else
                    {
                        j =0;
                        buffer2[i+addSize] = buffer[i];
                        buffer2[i+addSize+1] = coefficient;
                        addSize++;
                    }
                }
            }
            dataSize = dataSize + addSize ;
            free(buffer);
            buffer =buffer2;
            break;
        defualt:
            // just play the original wave file
            printf("dataSize=%d  \n",dataSize);
            buffer = (char *)malloc( dataSize * sizeof(char )) ;
            fread(buffer, 1, dataSize, fp);       
            break ;
    }

    playWave16kChannel1(buffer,dataSize )  ;

    fclose(fp);  
    free(buffer);
    // if(buffer2 != NULL)
    // {
    //    free(buffer2);
    // }
}

void main_testChangeWaveData(int argc, char const *argv[])
{
     
    nero_8int * fileName3="/home/jty/nero/nero/src/data/sound/001102_L.wav";
    int i;
    // printf("%d\n",sizeof( nero_s16int ) );
    // printf("%d\n",sizeof( nero_us16int ) );

    // wavePlayInit(  fileName3 ,STRETCH,0.7);


    #ifdef Nero_DeBuging08_09_17_
    for(i=0;i<15;i++)
    {
       wavePlayInit(  fileName3 ,STRETCH, (float)i  * 0.1 * 2);
    }
    #endif 
// nero_s16int
    #ifdef Nero_DeBuging08_09_17
    // for(i=0;i<15;i++)
    {
       wavePlayInit(  fileName3 ,INTERPOLATION, 0);
    }
    #endif 
    return 0;
}
void main_create_pic(int argc, char const *argv[])
{
    /* code */
    int i;
    nero_8int * fileName3="/home/jty/nero/nero/src/data/sound/wave.csv";
    char *fName;

    // 
 

    // for(i=1;i>=0;i++)
    for(i=1;i==1;i++)
    {
        fName = getLineInFile( fileName3,i);//返回获取的字符串
        if(fName != NULL  &&  strlen(fName) > 2)
        {
            sprintf(file_path_getcwd,"/data/sound/%s",fName);
            printf(" :%s\n",file_path_getcwd );
              // playWaveFromDB(fName);
              create_pic_forWave(  file_path_getcwd);


        }
        else
        {
            break;

        }
    }
    return 0;
}
void main(int argc, char const *argv[])
{
    /* code */
    int i;
    nero_8int * fileName3="/home/jty/nero/nero/src/data/sound/001101_L2wav";
    char *fName;

    // 
 

      modifyWaveData(  fileName3 ,  STRETCH,  2);

    return 0;
}