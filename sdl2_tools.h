#ifndef SDL2_HELP_LIBRARY_SECURITY_H
#define SDL2_HELP_LIBRARY_SECURITY_H

#include "_s2t_tools.h"	// CustomVector class

//declarations:
//============================================================================================
double cSDL_timer_high_precision();
int32_t cSDL_load_streaming_texture(const char* const filename,SDL_Texture** output_texture,uint8_t** output_pixel_array,SDL_Renderer* render,uint32_t force_bytes_per_pixel,SDL_PixelFormatEnum force_pixelformat);
class cSDL_FpsControl;
class cSDL_MicroTimer;
class cSDL_Anim3D_Yaxis;
class cSDL_Anim3D_Xaxis;
class cSDL_Anim3D_Xaxis_border;
class cSDL_Anim3D_Yaxis_border;
class cSDL_ButtonsManager;
class cSDL_ManualSlider;
class cSDL_AutomaticSlider;
//============================================================================================




// returns time in miliseconds (and even in nanoseconds before decimal numbers) which counts time from booting up OS;
//  it's a more precise form of the SDL_GetTickCount() function;
double cSDL_timer_high_precision()
{
    static Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 counter = SDL_GetPerformanceCounter();
    return (double)((double)counter/(double)frequency)*1000.0;
}


// #include <SDL2/SDL.h>
// #include <SDL2/SDL_image.h>
// loads file and creates texture & pixels array optimalized for frequent change of pixels by for example "SDL_UpdateTexture"; texture & array will be created without padding;
// "filename" file name of image to load;
// "output_texture" returns pointer to allocated and created texture (remember to destroy it afterwards by using func SDL_DestroyTexture());
// "output_pixel_array" returns pointer to allocated and created array of pixels that are in "output_texture" without padding;
//      remember to deallocate this array by using function free() and don't pass not allocated first pointer to this function; only second pointer in this variable shouldn't be allocated;
// "render" your window render;
// "force_bytes_per_pixel" forced number of channels u want per pixel; for example RGBA has 4 channels (4bytes) per pixel; RGB has 3 channels and etc.;
//      leave it 0 if don't care about number of bytes per pixel and want function to automatically guess the best choice for specific texture;
//      variable can be 3 or 4; SDL_Texture* doesn't make 2 or 1 byte long pixel formats;
//      this is ignored if u set variable "force_pixelformat"; and if there is no supported pixel formats by render that has specific numbers of
//      bytes of pixel,then for 4bytes long option will be setted SDL_PIXELFORMAT_ARGB8888 and for 3bytes long option SDL_PIXELFORMAT_RGB24;
// "force_pixelformat" force specific format for texture to create; leave it SDL_PIXELFORMAT_UNKNOWN if u want to create texture with the best format for ur render automatically;
// if render doesn't support any 3 or 4 bytes per pixel formats,then forcefully will be created texture with the format SDL_PIXELFORMAT_ARGB8888;
//
// returns 0 if no error occurs; returns -1 if there is problem with loading image from filename; returns -2 if variable/variables passed to func are incorrect;
// returns -3 if failed to create texture; returns -4 if occurs error while conferting surface to different format by function "SDL_ConvertSurfaceFormat";
// returns -5 if occurs error while receiving info about best pixel formats for render by using func "SDL_GetRendererInfo";
// returns -6 if there's not enough memory or there is any other problem with allocating memory for "output_texture" in function "malloc()";
// if function exits with error,then u don't have to deallocate any of the variables 'cause all of the variables will be not allocated;
// run "SDL_QueryTexture()" func after this function to get "w","h" and eventually "format"; pitch is always width*bytes_per_pixel;
int32_t cSDL_load_streaming_texture(const char* const filename,SDL_Texture** output_texture,uint8_t** output_pixel_array,SDL_Renderer* render,uint32_t force_bytes_per_pixel = 4,SDL_PixelFormatEnum force_pixelformat = SDL_PIXELFORMAT_UNKNOWN)
{
    if(filename==NULL||render==NULL||output_texture==NULL||output_pixel_array==NULL) return -2;

    SDL_Surface* tmp = IMG_Load(filename);
    if(tmp==NULL) return -1;

    SDL_Surface* img = NULL;
    uint32_t bytes_per_pixel;
    Uint32 chosen_format = 0;
    if(force_pixelformat!=0)
    {
        img = SDL_ConvertSurfaceFormat(tmp,force_pixelformat,0);
        SDL_FreeSurface(tmp);
        if(img==NULL) return -4;
        bytes_per_pixel = SDL_BYTESPERPIXEL(force_pixelformat);
        chosen_format = force_pixelformat;
    }
    else
    {
        SDL_RendererInfo info;
        if(SDL_GetRendererInfo(render,&info)!=0)
        {
            SDL_FreeSurface(tmp);
            return -5;
        }

        for (int i = 0; i!=info.num_texture_formats; i++)
        {
            if(force_bytes_per_pixel!=0)
            {
                if(force_bytes_per_pixel==SDL_BYTESPERPIXEL(info.texture_formats[i]))
                {
                    chosen_format = info.texture_formats[i];
                    bytes_per_pixel = force_bytes_per_pixel;
                    break;
                }
            }
            else
            {
                bytes_per_pixel = SDL_BYTESPERPIXEL(info.texture_formats[i]);
                if(bytes_per_pixel==4||bytes_per_pixel==3)
                {
                    chosen_format = info.texture_formats[i];
                    break;
                }
            }
        }
        if(chosen_format==0)
        {
            chosen_format = (force_bytes_per_pixel==3) ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_ARGB8888;
            bytes_per_pixel = SDL_BYTESPERPIXEL(chosen_format);
        }

        img = SDL_ConvertSurfaceFormat(tmp,chosen_format,0);
        SDL_FreeSurface(tmp);
        if(img==NULL) return -4;

    }


    int32_t w = img->w, h = img->h;
    int32_t expected_pitch = w*bytes_per_pixel;

    uint8_t* buffer = (uint8_t*)malloc(w*h*bytes_per_pixel);
    if (buffer==NULL)
    {
        SDL_FreeSurface(img);
        return -6;
    }
    if (img->pitch!=expected_pitch)
    {
        uint8_t* src = (uint8_t*)img->pixels;
        for(int32_t y = 0; y!=h; y++) memcpy(&buffer[y*w*bytes_per_pixel],src+(y*img->pitch),expected_pitch);
    }
    else memcpy(buffer,img->pixels,w*h*bytes_per_pixel);
    *output_pixel_array = (void*)buffer;

    SDL_FreeSurface(img);

    *output_texture = SDL_CreateTexture(render,chosen_format,SDL_TEXTUREACCESS_STREAMING,w,h);
    if(*output_texture==NULL)
    {
        free(*output_pixel_array);
        return -3;
    }

    return 0;
}

// #include <SDL2/SDL.h>
// #include <stdint.h>
//      class for purpose of stabilize and control fps of a program;
// i think it's possible to make high-precision nano "sleep()" mechanism without "busy-wait" method that consumes high amount of CPU power
//      and this class has been created as proof of it; class uses high frequency OS timer to calculate the rest of time that will be taken
//      into account when measuring future delays time; the results of that are stabilized frames in precision of nano-seconds;
//      if 1 frame is a little slower or faster,then future frames are faster/slower based on past frames but if the program doesn't cath up
//      to default delay time,then frame/frames are skipped results in sharp lag; so if u set up program to has 300 fps,then u will not see fps drops
//      beetwen 150-300 but only below 150 'cause in real scenario where u should have 260 frames per seconds for example,this class
//      stabilizes future frames makes future frame/frames has lower delay so as whole u will get stable 300 frames per second;
//      so class is good for programs where u don't expect often fps drops below expected_fps/2;
// this class needs more testing and cheking in good environment to check if class gives perfect results in every scenario;
class cSDL_FpsControl
{
    Uint64 frequency_of_timer;

    double fps_end;
    double fps_start;
    uint32_t expected_fps;
    float default_fps_delay;
    double time_difference;

    double actual_fps;

public:
    cSDL_FpsControl(uint32_t fps);

    void end_frame();     //add at the end of loop;
    void start_frame();   //add at the the start of loop;
    void change_fps(uint32_t fps);

    //the variable "actual_fps" will be updated after call of the function "end_frame()";
    double get_actual_fps() const;
    uint32_t get_expected_fps() const;

};

cSDL_FpsControl::cSDL_FpsControl(uint32_t fps = 60)
{
    this->expected_fps = fps;
    this->actual_fps = fps;
    this->default_fps_delay = (double)1000.0/fps;
    this->frequency_of_timer = SDL_GetPerformanceFrequency();
    this->time_difference = 0.0;
    return;
}

void cSDL_FpsControl::end_frame()
{
    double actual_timer = ((double)SDL_GetPerformanceCounter()/this->frequency_of_timer)*1000.0;

    this->fps_end = (actual_timer-this->fps_start)+this->time_difference;

    this->time_difference = 0.0;
    if(this->default_fps_delay>this->fps_end)
    {
        this->actual_fps = (double)this->expected_fps;

        double actual_delay = this->default_fps_delay-this->fps_end;
        SDL_Delay((uint32_t)actual_delay);
        actual_timer = (((double)SDL_GetPerformanceCounter()/this->frequency_of_timer)*1000.0)-actual_timer;

        this->time_difference = actual_timer-actual_delay;
    }
    else
    {
        this->time_difference = this->fps_end-this->default_fps_delay;

        if(this->time_difference>this->default_fps_delay)
        {
            this->actual_fps = (double)1000.0/this->fps_end;
            this->time_difference = 0.0;

        } else this->actual_fps = (double)this->expected_fps;
    }

    return;
}

void cSDL_FpsControl::start_frame()
{
    fps_start = ((double)SDL_GetPerformanceCounter()/this->frequency_of_timer)*1000.0;
    return;
}

void cSDL_FpsControl::change_fps(uint32_t fps)
{
    if(fps==0) fps = 1;
    this->expected_fps = fps;
    this->default_fps_delay = (double)1000.0/fps;
    this->time_difference = 0.0;
    return;
}

double cSDL_FpsControl::get_actual_fps() const
{
    return this->actual_fps;
}
uint32_t cSDL_FpsControl::get_expected_fps() const
{
    return this->expected_fps;
}



// #include <SDL2/SDL.h>
// #include <stdint.h>
//      timer in precision to microseconds;
class cSDL_MicroTimer
{
    uint64_t internal_timer;
    uint64_t past_time;
    uint64_t previous_time_rest;

    uint64_t frequency;

public:

    cSDL_MicroTimer(){this->frequency = SDL_GetPerformanceFrequency(); this->internal_timer = 0; this->previous_time_rest = 0; return;}

// 1000 microseconds == 1 milisecond;
    void set_internal_timer(uint64_t delay_in_microseconds);

// returns 0 if internal timer is still running or if "internal_timer" == 0 (run function "set_internal_timer() with variable greater than 0);
// returns number of times timer ends his job and then resets accumulated number to 0;
    uint64_t run_timer();

// resets accumulated cycles of time when timer finishes his job;
    void reset_accumulated_timer();
};

void cSDL_MicroTimer::set_internal_timer(uint64_t delay_in_microseconds)
{
    if(delay_in_microseconds==0) return;
    this->internal_timer = delay_in_microseconds;
    this->past_time = (uint64_t)(((double)SDL_GetPerformanceCounter()/(double)this->frequency)*1000000);
    return;
}

uint64_t cSDL_MicroTimer::run_timer()
{
    if(this->internal_timer==0) return 0;
    uint64_t actual_time = (uint64_t)(((double)SDL_GetPerformanceCounter()/(double)this->frequency)*1000000);
    uint64_t accumulated_delay = (actual_time-this->past_time)+this->previous_time_rest;

    if(accumulated_delay>=this->internal_timer)
    {
        uint64_t whole_numbers = accumulated_delay/this->internal_timer;
        this->previous_time_rest = (accumulated_delay-(whole_numbers*this->internal_timer));
        this->past_time = actual_time;

        return whole_numbers;
    }

    return 0;
}

void cSDL_MicroTimer::reset_accumulated_timer()
{
    this->past_time = (uint64_t)(((double)SDL_GetPerformanceCounter()/(double)this->frequency)*1000000);
    this->previous_time_rest = 0;
    return;
}

//      cSDL class for 3D animation that flips around Y axis;
// #include <SDL/SDL2.h>
// #include <math.h>
// #include <stdint.h>
class cSDL_Anim3D_Yaxis
{
public:
    enum AnimationSpeedType{SPEED_LINEAR,SPEED_COS};
    enum ScalingType{CENTERED_SCALING,RAW_SCALING};

private:
    int32_t _original_images_h;
    int32_t _original_images_w;
    SDL_Texture* _front_image;
    SDL_Texture* _back_image;
    float _speed;
    AnimationSpeedType _speed_type;

//  "_main_execute_animation()" mathematic variables;
    bool _half_flip;
    bool _whole_flip;
    SDL_Texture* _pointer_image;
    double _rotate_y_variable;
    bool _direction;

// "execute_animation()" & "execute_animation_draw()" mathematic variables;
    ScalingType _scaling_type;
    float _scaling;

public:
// "front_image" pointer to the front of the main image;
// "back_image" pointer to the back of the main image;
// "original_images_h" height of images in "front_image" and "back_image"; both images should have the same height;
// "original_images_w" width of images in "front_image" and "back_image"; both images should have the same width;
// "speed" speed of animation in float; number should be above 0.0;
// "speed_type" speed type;
// while using this class,u shouldn't free memory from pointers passed to "front_image","back_image";
//      also class doesn't free passed pointers so u have to do it yourself;
    cSDL_Anim3D_Yaxis(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                      float speed,AnimationSpeedType speed_type);

//      that's main function; it's should be executed for every frame in program;
// "io_rect" it's main variable on which class operates; "w" and "h" will be changed entirely after running this function;
//      to "x"/"y" are only added some numbers so there is a sense in setting beforehand these variables before passing them to this func;
// "actual_fps" should have actual updated application fps; shouldn't be below 1;
//      this variable is here for synchronization reasons for speeding up or down animation; variable can be static if u want;
// returns changed "io_rect"; in specific: to "y" & "x" are added some number,"h" and "w" are totaly changed;
// also returns pointer to "out_image_pointer" of texture that should be displayed in animation:
    void execute_animation(SDL_Texture** out_image_pointer,SDL_Rect* io_rect,uint32_t actual_fps);
//      it's the same function as "execute_animation" except it's additionally draws animation into render;
    void execute_animation_draw(SDL_Rect* io_rect,uint32_t actual_fps,SDL_Renderer* render);

//      changes "original_images_h" & "original_images_w";
// if u don't want to change some variable,just place there -1;
    void change_images_original_w_h(int32_t original_images_h,int32_t original_images_w);

//      adds number to actual scalling variable;
    void change_scaling_dynamically(float scaling);
//      swaps actual scaling variable inside class with this one;
// "scaling" should be above 0.0;
    void change_scaling(float scaling);
    void change_scaling_type(ScalingType scaling_type);

// number should be above 0.0;
    void change_speed(float speed);
    void change_speed_dynamically(float speed);
// if "left_or_right" == 0 then direction will be left; else if 1 then will be right;
    void change_direction(bool left_or_right);

    void change_image_front(SDL_Texture* front_image);
    void change_image_back(SDL_Texture* back_image);

// returns stage of half flips;
//      returns 0 if animation is at first stage;
//      returns 1 if animation is at second stage;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_half_flip_stage() const;

// returns stage of whole flips;
//      returns 0 if animation is at stage where
//          "image_front" is as front at default;
//      returns 1 if animation is at stage where "image_back" is as front at default;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_whole_flip_stage() const;

private:
//      main tool of "executa_animation" & "executa_animation_draw" functions;
    void _main_execute_animation(SDL_Rect* _io_rect,uint32_t _actual_fps);

// this function is universal and can be used separately without class;
//      3D animation designed to use with SDL_Texture and SDL_Rect;
// "rotate_y" should be from 0.0 to max 1.0;
// "io_y" can be 0 when passed to function; outputs new y of texture by adding new value to variable;
// "io_h" should have original height in pixels of texture when passed to function; it outputs and overwrites height of texture in variable;
    void _cSDL_anim3D_vertical_flip(int32_t& io_y,int32_t& io_h,float rotate_y);


};

cSDL_Anim3D_Yaxis::cSDL_Anim3D_Yaxis(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                                     float speed,AnimationSpeedType speed_type)
{
    this->_front_image = front_image;
    this->_back_image = back_image;
    this->_original_images_h = original_images_h;
    this->_original_images_w = original_images_w;
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    this->_speed_type = speed_type;

    this->_half_flip = 0;
    this->_whole_flip = 0;
    this->_pointer_image = front_image;
    this->_rotate_y_variable = 0.0;
    this->_direction = 1;

    this->_scaling_type = ScalingType::CENTERED_SCALING;
    this->_scaling = 1.0;

    return;
}

void cSDL_Anim3D_Yaxis::execute_animation(SDL_Texture** out_image_pointer,SDL_Rect* io_rect,uint32_t actual_fps)
{
    if(this->_scaling_type==ScalingType::RAW_SCALING)
    {
        io_rect->h = this->_original_images_h*this->_scaling;
        io_rect->w = this->_original_images_w*this->_scaling;
    }
    else if(this->_scaling_type==ScalingType::CENTERED_SCALING)
    {
        io_rect->h = this->_original_images_h*this->_scaling;
        io_rect->w = this->_original_images_w*this->_scaling;

        io_rect->x+=(this->_original_images_w-(io_rect->w))/2;
        io_rect->y+=(this->_original_images_h-(io_rect->h))/2;
    }

    this->_main_execute_animation(io_rect,actual_fps);
    *out_image_pointer = this->_pointer_image;


    return;
}

void cSDL_Anim3D_Yaxis::execute_animation_draw(SDL_Rect* io_rect,uint32_t actual_fps,SDL_Renderer* render)
{
    SDL_Texture* out_image_pointer;
    this->execute_animation(&out_image_pointer,io_rect,actual_fps);
    SDL_RenderCopy(render,out_image_pointer,NULL,io_rect);
    return;
}

void cSDL_Anim3D_Yaxis::change_images_original_w_h(int32_t original_images_h,int32_t original_images_w)
{
    if(original_images_h!=-1) this->_original_images_h = original_images_h;
    if(original_images_w!=-1) this->_original_images_w = original_images_w;
    return;
}

void cSDL_Anim3D_Yaxis::change_scaling_dynamically(float scaling)
{
    this->_scaling+=scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Yaxis::change_scaling(float scaling)
{
    this->_scaling = scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Yaxis::change_scaling_type(ScalingType scaling_type)
{
    this->_scaling_type = scaling_type;
    return;
}

void cSDL_Anim3D_Yaxis::change_speed(float speed)
{
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    return;
}

void cSDL_Anim3D_Yaxis::change_speed_dynamically(float speed)
{
    this->_speed+=speed;
    if(this->_speed<0.0) this->_speed = 0.0;
    return;
}

void cSDL_Anim3D_Yaxis::change_direction(bool left_or_right)
{
    this->_direction = left_or_right;
    return;
}

void cSDL_Anim3D_Yaxis::change_image_front(SDL_Texture* front_image)
{
    this->_front_image = front_image;
    return;
}

void cSDL_Anim3D_Yaxis::change_image_back(SDL_Texture* back_image)
{
    this->_back_image = back_image;
    return;
}

bool cSDL_Anim3D_Yaxis::get_half_flip_stage() const
{
    return this->_half_flip;
}

bool cSDL_Anim3D_Yaxis::get_whole_flip_stage() const
{
    return this->_whole_flip;
}

void cSDL_Anim3D_Yaxis::_main_execute_animation(SDL_Rect* _io_rect,uint32_t _actual_fps)
{
    if(_actual_fps==0) _actual_fps = 1;

    float rotate_y;

//  i know that code is repeating but it's just easier to follow for me this way;
    if(this->_speed_type==AnimationSpeedType::SPEED_COS)
    {
        if(this->_direction==1)
        {
            this->_rotate_y_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if((this->_rotate_y_variable/M_PI)>1.0)
            {
                this->_rotate_y_variable-=M_PI;
                if(this->_rotate_y_variable>M_PI) this->_rotate_y_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_y_variable/M_PI)>=0.5)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }
        }
        else
        {
            this->_rotate_y_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_y_variable<0.0)
            {
                this->_rotate_y_variable+=M_PI;
                if(this->_rotate_y_variable<0.0) this->_rotate_y_variable = M_PI; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_y_variable/M_PI)<0.5)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }

        rotate_y = fabsf(cosf(this->_rotate_y_variable)); // 1 -> 0 -> 1 (cosf depends on M_PI);
    }
    else
    {
        if(this->_direction==1)
        {
            this->_rotate_y_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_y_variable>2.0)
            {
                this->_rotate_y_variable-=2.0;
                if(this->_rotate_y_variable>2.0) this->_rotate_y_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_y_variable)>=1.0)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        else
        {
            this->_rotate_y_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_y_variable<0.0)
            {
                this->_rotate_y_variable+=2.0;
                if(this->_rotate_y_variable<0.0) this->_rotate_y_variable = 2.0; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_y_variable)<1.0)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        rotate_y = fabsf(1.0-this->_rotate_y_variable); // 1 -> 0 -> 1 (fabsf converts negative numbers to posiiive);
    }

    this->_cSDL_anim3D_vertical_flip(_io_rect->y,_io_rect->h,rotate_y);

    return;
}


void cSDL_Anim3D_Yaxis::_cSDL_anim3D_vertical_flip(int32_t& io_y,int32_t& io_h,float rotate_y)
{
    if(rotate_y>1.0) rotate_y = 1.0;
    else if (rotate_y<(float)(1.0/io_h))  rotate_y = 0.0;

    const int32_t out_h = (int32_t)(io_h*rotate_y);  // final height of main image;
    const int32_t off_y = (int32_t)((io_h-out_h)/2); // position offset of main image;

    io_y+=off_y;
    io_h = out_h;

    return;
}

//      cSDL class for 3D animation that flips around X axis;
// #include <SDL/SDL2.h>
// #include <math.h>
// #include <stdint.h>
class cSDL_Anim3D_Xaxis
{
public:
    enum AnimationSpeedType{SPEED_LINEAR,SPEED_COS};
    enum ScalingType{CENTERED_SCALING,RAW_SCALING};

private:
    int32_t _original_images_h;
    int32_t _original_images_w;
    SDL_Texture* _front_image;
    SDL_Texture* _back_image;
    float _speed;
    AnimationSpeedType _speed_type;

//  "_main_execute_animation()" mathematic variables;
    bool _half_flip;
    bool _whole_flip;
    SDL_Texture* _pointer_image;
    double _rotate_x_variable;
    bool _direction;

// "execute_animation()" & "execute_animation_draw()" mathematic variables;
    ScalingType _scaling_type;
    float _scaling;

public:
// "front_image" pointer to the front of the main image;
// "back_image" pointer to the back of the main image;
// "original_images_h" height of images in "front_image" and "back_image"; both images should have the same height;
// "original_images_w" width of images in "front_image" and "back_image"; both images should have the same width;
// "speed" speed of animation in float; number should be above 0.0;
// "speed_type" speed type;
// while using this class,u shouldn't free memory from pointers passed to "front_image","back_image";
//      also class doesn't free passed pointers so u have to do it yourself;
    cSDL_Anim3D_Xaxis(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                      float speed,AnimationSpeedType speed_type);

//      that's main function; it's should be executed for every frame in program;
// "io_rect" it's main variable on which class operates; "w" and "h" will be changed entirely after running this function;
//      to "x"/"y" are only added some numbers so there is a sense in setting beforehand these variables before passing them to this func;
// "actual_fps" should have actual updated application fps; shouldn't be below 1;
//      this variable is here for synchronization reasons for speeding up or down animation; variable can be static if u want;
// returns changed "io_rect"; in specific: to "y" & "x" are added some number,"h" and "w" are totaly changed;
// also returns pointer to "out_image_pointer" of texture that should be displayed in animation:
    void execute_animation(SDL_Texture** out_image_pointer,SDL_Rect* io_rect,uint32_t actual_fps);
//      it's the same function as "execute_animation" except it's additionally draws animation into render;
    void execute_animation_draw(SDL_Rect* io_rect,uint32_t actual_fps,SDL_Renderer* render);

//      changes "original_images_h" & "original_images_w";
// if u don't want to change some variable,just place there -1;
    void change_images_original_w_h(int32_t original_images_h,int32_t original_images_w);

//      adds number to actual scalling variable;
    void change_scaling_dynamically(float scaling);
//      swaps actual scaling variable inside class with this one;
// "scaling" should be above 0.0;
    void change_scaling(float scaling);
    void change_scaling_type(ScalingType scaling_type);

// number should be above 0.0;
    void change_speed(float speed);
    void change_speed_dynamically(float speed);
// if "left_or_right" == 0 then direction will be left; else if 1 then will be right;
    void change_direction(bool left_or_right);

    void change_image_front(SDL_Texture* front_image);
    void change_image_back(SDL_Texture* back_image);

// returns stage of half flips;
//      returns 0 if animation is at first stage;
//      returns 1 if animation is at second stage;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_half_flip_stage() const;

// returns stage of whole flips;
//      returns 0 if animation is at stage where
//          "image_front" is as front at default;
//      returns 1 if animation is at stage where "image_back" is as front at default;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_whole_flip_stage() const;

private:
//      main tool of "executa_animation" & "executa_animation_draw" functions;
    void _main_execute_animation(SDL_Rect* _io_rect,uint32_t _actual_fps);

// this function is universal and can be used separately without class;
//      3D animation designed to use with SDL_Texture and SDL_Rect;
// "rotate_x" should be from 0.0 to max 1.0;
// "io_x" can be 0 when passed to function; outputs new x of texture by adding new value to variable;
// "io_w" should have original width in pixels of texture when passed to function; it outputs and overwrites width of texture in variable;
    void _cSDL_anim3D_horizontal_flip(int32_t& io_x,int32_t& io_w,float rotate_x);


};

cSDL_Anim3D_Xaxis::cSDL_Anim3D_Xaxis(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                                     float speed,AnimationSpeedType speed_type)
{
    this->_front_image = front_image;
    this->_back_image = back_image;
    this->_original_images_h = original_images_h;
    this->_original_images_w = original_images_w;
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    this->_speed_type = speed_type;

    this->_half_flip = 0;
    this->_whole_flip = 0;
    this->_pointer_image = front_image;
    this->_rotate_x_variable = 0.0;
    this->_direction = 1;

    this->_scaling_type = ScalingType::CENTERED_SCALING;
    this->_scaling = 1.0;

    return;
}

void cSDL_Anim3D_Xaxis::execute_animation(SDL_Texture** out_image_pointer,SDL_Rect* io_rect,uint32_t actual_fps)
{
    if(this->_scaling_type==ScalingType::RAW_SCALING)
    {
        io_rect->h = this->_original_images_h*this->_scaling;
        io_rect->w = this->_original_images_w*this->_scaling;
    }
    else if(this->_scaling_type==ScalingType::CENTERED_SCALING)
    {
        io_rect->h = this->_original_images_h*this->_scaling;
        io_rect->w = this->_original_images_w*this->_scaling;

        io_rect->x+=(this->_original_images_w-(io_rect->w))/2;
        io_rect->y+=(this->_original_images_h-(io_rect->h))/2;
    }

    this->_main_execute_animation(io_rect,actual_fps);
    *out_image_pointer = this->_pointer_image;


    return;
}

void cSDL_Anim3D_Xaxis::execute_animation_draw(SDL_Rect* io_rect,uint32_t actual_fps,SDL_Renderer* render)
{
    SDL_Texture* out_image_pointer;
    this->execute_animation(&out_image_pointer,io_rect,actual_fps);
    SDL_RenderCopy(render,out_image_pointer,NULL,io_rect);
    return;
}

void cSDL_Anim3D_Xaxis::change_images_original_w_h(int32_t original_images_h,int32_t original_images_w)
{
    if(original_images_h!=-1) this->_original_images_h = original_images_h;
    if(original_images_w!=-1) this->_original_images_w = original_images_w;
    return;
}

void cSDL_Anim3D_Xaxis::change_scaling_dynamically(float scaling)
{
    this->_scaling+=scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Xaxis::change_scaling(float scaling)
{
    this->_scaling = scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Xaxis::change_scaling_type(ScalingType scaling_type)
{
    this->_scaling_type = scaling_type;
    return;
}

void cSDL_Anim3D_Xaxis::change_speed(float speed)
{
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    return;
}

void cSDL_Anim3D_Xaxis::change_speed_dynamically(float speed)
{
    this->_speed+=speed;
    if(this->_speed<0.0) this->_speed = 0.0;
    return;
}

void cSDL_Anim3D_Xaxis::change_direction(bool left_or_right)
{
    this->_direction = left_or_right;
    return;
}

void cSDL_Anim3D_Xaxis::change_image_front(SDL_Texture* front_image)
{
    this->_front_image = front_image;
    return;
}

void cSDL_Anim3D_Xaxis::change_image_back(SDL_Texture* back_image)
{
    this->_back_image = back_image;
    return;
}

bool cSDL_Anim3D_Xaxis::get_half_flip_stage() const
{
    return this->_half_flip;
}

bool cSDL_Anim3D_Xaxis::get_whole_flip_stage() const
{
    return this->_whole_flip;
}

void cSDL_Anim3D_Xaxis::_main_execute_animation(SDL_Rect* _io_rect,uint32_t _actual_fps)
{
    if(_actual_fps==0) _actual_fps = 1;

    float rotate_x;

//  i know that code is repeating but it's just easier to follow for me this way;
    if(this->_speed_type==AnimationSpeedType::SPEED_COS)
    {
        if(this->_direction==1)
        {
            this->_rotate_x_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if((this->_rotate_x_variable/M_PI)>1.0)
            {
                this->_rotate_x_variable-=M_PI;
                if(this->_rotate_x_variable>M_PI) this->_rotate_x_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_x_variable/M_PI)>=0.5)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }
        }
        else
        {
            this->_rotate_x_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_x_variable<0.0)
            {
                this->_rotate_x_variable+=M_PI;
                if(this->_rotate_x_variable<0.0) this->_rotate_x_variable = M_PI; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_x_variable/M_PI)<0.5)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }

        rotate_x = fabsf(cosf(this->_rotate_x_variable)); // 1 -> 0 -> 1 (cosf depends on M_PI);
    }
    else
    {
        if(this->_direction==1)
        {
            this->_rotate_x_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_x_variable>2.0)
            {
                this->_rotate_x_variable-=2.0;
                if(this->_rotate_x_variable>2.0) this->_rotate_x_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_x_variable)>=1.0)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        else
        {
            this->_rotate_x_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_x_variable<0.0)
            {
                this->_rotate_x_variable+=2.0;
                if(this->_rotate_x_variable<0.0) this->_rotate_x_variable = 2.0; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                }
                else
                {
                    this->_whole_flip = 0;
                }
            }
            else if((this->_rotate_x_variable)<1.0)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        rotate_x = fabsf(1.0-this->_rotate_x_variable); // 1 -> 0 -> 1 (fabsf converts negative numbers to posiiive);
    }

    this->_cSDL_anim3D_horizontal_flip(_io_rect->x,_io_rect->w,rotate_x);

    return;
}


void cSDL_Anim3D_Xaxis::_cSDL_anim3D_horizontal_flip(int32_t& io_x,int32_t& io_w,float rotate_x)
{
    if(rotate_x>1.0) rotate_x = 1.0;
    else if (rotate_x<(float)(1.0f/io_w)) rotate_x = 0.0;

    const int32_t out_w = (int32_t)(io_w*rotate_x);    // final width of main image;
    const int32_t off_x = (int32_t)((io_w-out_w)/2);	// position offset of main image;

    io_x+=off_x;
    io_w = out_w;

    return;
}


//      cSDL class for 3D animation that flips around X axis;
// #include <SDL/SDL2.h>
// #include <math.h>
// #include <stdint.h>
class cSDL_Anim3D_Xaxis_border
{
public:
    enum AnimationSpeedType{SPEED_LINEAR,SPEED_COS};
    enum ScalingType{CENTERED_SCALING,RAW_SCALING};

private:
    int32_t _original_images_h;
    int32_t _original_images_w;
    int32_t _original_borders_h;
    int32_t _original_borders_w;
    SDL_Texture* _front_image;
    SDL_Texture* _back_image;
    SDL_Texture* _front_border;
    SDL_Texture* _back_border;
    float _speed;
    AnimationSpeedType _speed_type;

//  "_main_execute_animation()" mathematic variables;
    bool _half_flip;
    bool _whole_flip;
    SDL_Texture* _pointer_image;
    SDL_Texture* _pointer_border;
    double _rotate_x_variable;
    bool _direction;

// "execute_animation()" & "execute_animation_draw()" mathematic variables;
    ScalingType _scaling_type;
    float _scaling;

public:
// "front_image" pointer to the front of the main image;
// "back_image" pointer to the back of the main image;
// "original_images_h" height of images in "front_image" and "back_image"; both images should have the same height;
// "original_images_w" width of images in "front_image" and "back_image"; both images should have the same width;
// "front_border" pointer to the right of the border image;
// "back_border" pointer to the left of the border image;
// "original_borders_h" height of images in "front_border" and "back_border"; both images should have the same height;
// "original_borders_w" width of images in "front_border" and "back_border"; both images should have the same width;
// "speed" speed of animation in float; number should be above 0.0;
// "speed_type" speed type;
// while using this class,u shouldn't free memory from pointers passed to "front_image","back_image","front_border","back_border";
//      also class doesn't free passed pointers so u have to do it yourself;
    cSDL_Anim3D_Xaxis_border(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                                SDL_Texture* front_border,SDL_Texture* back_border,int32_t original_borders_h,int32_t original_borders_w,
                                    float speed,AnimationSpeedType speed_type);

//      that's main function; it's should be executed for every frame in program;
// "io_rect_images" & "io_rect_borders" are main variables on which class operates; "w" and "h" will be changed entirely after running this function;
//      to "x" & "y" are only added some numbers so there is a sense in setting beforehand these variables before passing them to this func;
//      "x" & "y" of "io_rect_images" & "io_rect_borders" should be the same;
// "actual_fps" should have actual updated application fps; shouldn't be below 1;
//      this variable is here for synchronization reasons for speeding up or down animation; variable can be static if u want;
// returns changed "io_rect_images" and "io_rect_borders"; in specific: to "y" & "x" are added some number,"h" and "w" are totaly changed;
// also returns pointer to "out_image_pointer" and "out_border_pointer" of textures that should be displayed in animation:
    void execute_animation(SDL_Texture** out_image_pointer,SDL_Texture** out_border_pointer,SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps);
//      it's the same function as "execute_animation" except it's additionally draws animation into render;
    void execute_animation_draw(SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps,SDL_Renderer* render);

//      changes "original_images_h" & "original_images_w";
// if u don't want to change some variable,just place there -1;
    void change_images_original_w_h(int32_t original_images_h,int32_t original_images_w);
//      changes "original_borders_h" & "original_borders_w";
// if u don't want to change some variable,just place there -1;
    void change_borders_original_w_h(int32_t original_borders_h,int32_t original_borders_w);

//      adds number to actual scalling variable;
    void change_scaling_dynamically(float scaling);
//      swaps actual scaling variable inside class with this one;
// "scaling" should be above 0.0;
    void change_scaling(float scaling);
    void change_scaling_type(ScalingType scaling_type);

// number should be above 0.0;
    void change_speed(float speed);
    void change_speed_dynamically(float speed);
// if "left_or_right" == 0 then direction will be left; else if 1 then will be right;
    void change_direction(bool left_or_right);

    void change_image_front(SDL_Texture* front_image);
    void change_image_back(SDL_Texture* back_image);
    void change_border_front(SDL_Texture* front_border);
    void change_border_back(SDL_Texture* back_border);

// returns stage of half flips;
//      returns 0 if animation is at first stage;
//      returns 1 if animation is at second stage;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_half_flip_stage() const;

// returns stage of whole flips;
//      returns 0 if animation is at stage where
//          "image_front" is as front at default;
//      returns 1 if animation is at stage where "image_back" is as front at default;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_whole_flip_stage() const;

private:
//      main tool of "executa_animation" & "executa_animation_draw" functions;
    void _main_execute_animation(SDL_Rect* _io_rect_images,SDL_Rect* _io_rect_borders,uint32_t _actual_fps);

// this function is universal and can be used separately without class;
//      3D animation designed to use with SDL_Texture and SDL_Rect;
// "io_main_x" can be 0 when passed to function; outputs new x of texture by adding new value to variable;
// "io_main_w" should have original width in pixels of texture when passed to function; it outputs and overwrites width of texture in variable;
// "io_border_x" can be 0 when passed to function; outputs new x of border texture by adding new value to variable;
// "io_border_w" should have original width in pixels of border texture when passed to function; it outputs and overwrites width of border texture in variable;
//      "io_border_w" should be lower than "io_main_w" otherwise there will be problems;
// "half_flip" tells stage of animation;
// "rotate_x" should be from 0.0 to max 1.0;
void _cSDL_anim3D_horizontal_flip_border(int32_t& io_main_x,int32_t& io_main_w,
                                    int32_t& io_border_x,int32_t& io_border_w,bool half_flip,float rotate_x);


};

cSDL_Anim3D_Xaxis_border::cSDL_Anim3D_Xaxis_border(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                                SDL_Texture* front_border,SDL_Texture* back_border,int32_t original_borders_h,int32_t original_borders_w,
                                    float speed,AnimationSpeedType speed_type)
{
    this->_front_image = front_image;
    this->_back_image = back_image;
    this->_front_border = front_border;
    this->_back_border = back_border;
    this->_original_images_h = original_images_h;
    this->_original_images_w = original_images_w;
    this->_original_borders_h = original_borders_h;
    this->_original_borders_w = original_borders_w;
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    this->_speed_type = speed_type;

    this->_half_flip = 0;
    this->_whole_flip = 0;
    this->_pointer_image = front_image;
    this->_pointer_border = front_border;
    this->_rotate_x_variable = 0.0;
    this->_direction = 1;

    this->_scaling_type = ScalingType::CENTERED_SCALING;
    this->_scaling = 1.0;

    return;
}

void cSDL_Anim3D_Xaxis_border::execute_animation(SDL_Texture** out_image_pointer,SDL_Texture** out_border_pointer,SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps)
{
    if(this->_scaling_type==ScalingType::RAW_SCALING)
    {
        io_rect_images->h = this->_original_images_h*this->_scaling;
        io_rect_images->w = this->_original_images_w*this->_scaling;

        io_rect_borders->h = this->_original_borders_h*this->_scaling;
        io_rect_borders->w = this->_original_borders_w*this->_scaling;
    }
    else if(this->_scaling_type==ScalingType::CENTERED_SCALING)
    {
        io_rect_images->h = this->_original_images_h*this->_scaling;
        io_rect_images->w = this->_original_images_w*this->_scaling;
        io_rect_borders->h = this->_original_borders_h*this->_scaling;
        io_rect_borders->w = this->_original_borders_w*this->_scaling;

        io_rect_images->x+=(this->_original_images_w-(io_rect_images->w))/2;
        io_rect_images->y+=(this->_original_images_h-(io_rect_images->h))/2;
        io_rect_borders->x+=(this->_original_images_w-(io_rect_images->w))/2;
        io_rect_borders->y+=(this->_original_images_h-(io_rect_images->h))/2;
    }

    this->_main_execute_animation(io_rect_images,io_rect_borders,actual_fps);
    *out_image_pointer = this->_pointer_image;
    *out_border_pointer = this->_pointer_border;


    return;
}

void cSDL_Anim3D_Xaxis_border::execute_animation_draw(SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps,SDL_Renderer* render)
{
    SDL_Texture* out_image_pointer;
    SDL_Texture* out_border_pointer;
    this->execute_animation(&out_image_pointer,&out_border_pointer,io_rect_images,io_rect_borders,actual_fps);
    SDL_RenderCopy(render,out_image_pointer,NULL,io_rect_images);
    SDL_RenderCopy(render,out_border_pointer,NULL,io_rect_borders);
    return;
}

void cSDL_Anim3D_Xaxis_border::change_images_original_w_h(int32_t original_images_h,int32_t original_images_w)
{
    if(original_images_h!=-1) this->_original_images_h = original_images_h;
    if(original_images_w!=-1) this->_original_images_w = original_images_w;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_borders_original_w_h(int32_t original_borders_h,int32_t original_borders_w)
{
    if(original_borders_h!=-1) this->_original_borders_h = original_borders_h;
    if(original_borders_w!=-1) this->_original_borders_w = original_borders_w;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_scaling_dynamically(float scaling)
{
    this->_scaling+=scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_scaling(float scaling)
{
    this->_scaling = scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_scaling_type(ScalingType scaling_type)
{
    this->_scaling_type = scaling_type;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_speed(float speed)
{
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_speed_dynamically(float speed)
{
    this->_speed+=speed;
    if(this->_speed<0.0) this->_speed = 0.0;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_direction(bool left_or_right)
{
    this->_direction = left_or_right;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_image_front(SDL_Texture* front_image)
{
    this->_front_image = front_image;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_image_back(SDL_Texture* back_image)
{
    this->_back_image = back_image;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_border_front(SDL_Texture* front_border)
{
    this->_front_border = front_border;
    return;
}

void cSDL_Anim3D_Xaxis_border::change_border_back(SDL_Texture* back_border)
{
    this->_back_border = back_border;
    return;
}

bool cSDL_Anim3D_Xaxis_border::get_half_flip_stage() const
{
    return this->_half_flip;
}

bool cSDL_Anim3D_Xaxis_border::get_whole_flip_stage() const
{
    return this->_whole_flip;
}

void cSDL_Anim3D_Xaxis_border::_main_execute_animation(SDL_Rect* _io_rect_images,SDL_Rect* _io_rect_borders,uint32_t _actual_fps)
{
    if(_actual_fps==0) _actual_fps = 1;

    float rotate_x;

//  i know that code is repeating but it's just easier to follow for me this way;
    if(this->_speed_type==AnimationSpeedType::SPEED_COS)
    {
        if(this->_direction==1)
        {
            this->_rotate_x_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if((this->_rotate_x_variable/M_PI)>1.0)
            {
                this->_rotate_x_variable-=M_PI;
                if(this->_rotate_x_variable>M_PI) this->_rotate_x_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_x_variable/M_PI)>=0.5)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }
        }
        else
        {
            this->_rotate_x_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_x_variable<0.0)
            {
                this->_rotate_x_variable+=M_PI;
                if(this->_rotate_x_variable<0.0) this->_rotate_x_variable = M_PI; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_x_variable/M_PI)<0.5)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }

        rotate_x = fabsf(cosf(this->_rotate_x_variable)); // 1 -> 0 -> 1 (cosf depends on M_PI);
    }
    else
    {
        if(this->_direction==1)
        {
            this->_rotate_x_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_x_variable>2.0)
            {
                this->_rotate_x_variable-=2.0;
                if(this->_rotate_x_variable>2.0) this->_rotate_x_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_x_variable)>=1.0)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        else
        {
            this->_rotate_x_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_x_variable<0.0)
            {
                this->_rotate_x_variable+=2.0;
                if(this->_rotate_x_variable<0.0) this->_rotate_x_variable = 2.0; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_x_variable)<1.0)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        rotate_x = fabsf(1.0-this->_rotate_x_variable); // 1 -> 0 -> 1 (fabsf converts negative numbers to posiiive);
    }

    this->_cSDL_anim3D_horizontal_flip_border(_io_rect_images->x,_io_rect_images->w,_io_rect_borders->x,_io_rect_borders->w,this->_half_flip,rotate_x);

    return;
}


void cSDL_Anim3D_Xaxis_border::_cSDL_anim3D_horizontal_flip_border(int32_t& io_main_x,int32_t& io_main_w,
                                    int32_t& io_border_x,int32_t& io_border_w,bool half_flip,float rotate_x)
{
    if(rotate_x>1.0) rotate_x = 1.0;                         // "rotate_x" cannot be larger than 1;
    else if (rotate_x<(float)(1.0/io_main_w)) rotate_x = 0.0;   // just to be sure that "out_w_border" and "out_w" are 0;

    const int32_t out_w_border = (int32_t)(io_border_w-(io_border_w*rotate_x)); // final width of border;

    const int32_t out_w = (int32_t)(io_main_w*rotate_x); // final width of main image;
    const int32_t off_x = (int32_t)(((io_main_w-out_w)+out_w_border)/2);	// position offset of main image;



    // main_image
//==========================================================

    int32_t flip_offset;
    if(half_flip==0)            //  front side of the main image;
    {
        flip_offset = 0;
    }
    else                        //  back side of the main image;
    {
        flip_offset = out_w_border;
    }
    io_main_x+=off_x-flip_offset;
    io_main_w = out_w;


//==========================================================


    // border
//==========================================================

    int32_t off_x_border;
    if(half_flip==0)
    {
        off_x_border = (off_x-out_w_border);  // -- dangerous --

    }
    else off_x_border = ((off_x+out_w)-out_w_border);
    io_border_x+=off_x_border;
    io_border_w = out_w_border;
//==========================================================

    return;
}


//      cSDL class for 3D animation that flips around Y axis;
// #include <SDL/SDL2.h>
// #include <math.h>
// #include <stdint.h>
class cSDL_Anim3D_Yaxis_border
{
public:
    enum AnimationSpeedType{SPEED_LINEAR,SPEED_COS};
    enum ScalingType{CENTERED_SCALING,RAW_SCALING};

private:
    int32_t _original_images_h;
    int32_t _original_images_w;
    int32_t _original_borders_h;
    int32_t _original_borders_w;
    SDL_Texture* _front_image;
    SDL_Texture* _back_image;
    SDL_Texture* _front_border;
    SDL_Texture* _back_border;
    float _speed;
    AnimationSpeedType _speed_type;

//  "_main_execute_animation()" mathematic variables;
    bool _half_flip;
    bool _whole_flip;
    SDL_Texture* _pointer_image;
    SDL_Texture* _pointer_border;
    double _rotate_y_variable;
    bool _direction;

// "execute_animation()" & "execute_animation_draw()" mathematic variables;
    ScalingType _scaling_type;
    float _scaling;

public:
// "front_image" pointer to the front of the main image;
// "back_image" pointer to the back of the main image;
// "original_images_h" height of images in "front_image" and "back_image"; both images should have the same height;
// "original_images_w" width of images in "front_image" and "back_image"; both images should have the same width;
// "front_border" pointer to the right of the border image;
// "back_border" pointer to the left of the border image;
// "original_borders_h" height of images in "front_border" and "back_border"; both images should have the same height;
// "original_borders_w" width of images in "front_border" and "back_border"; both images should have the same width;
// "speed" speed of animation in float; number should be above 0.0;
// "speed_type" speed type;
// while using this class,u shouldn't free memory from pointers passed to "front_image","back_image","front_border","back_border";
//      also class doesn't free passed pointers so u have to do it yourself;
    cSDL_Anim3D_Yaxis_border(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                                SDL_Texture* front_border,SDL_Texture* back_border,int32_t original_borders_h,int32_t original_borders_w,
                                    float speed,AnimationSpeedType speed_type);

//      that's main function; it's should be executed for every frame in program;
// "io_rect_images" & "io_rect_borders" are main variables on which class operates; "w" and "h" will be changed entirely after running this function;
//      to "x" & "y" are only added some numbers so there is a sense in setting beforehand these variables before passing them to this func;
//      "x" & "y" of "io_rect_images" & "io_rect_borders" should be the same;
// "actual_fps" should have actual updated application fps; shouldn't be below 1;
//      this variable is here for synchronization reasons for speeding up or down animation; variable can be static if u want;
// returns changed "io_rect_images" and "io_rect_borders"; in specific: to "y" & "x" are added some number,"h" and "w" are totaly changed;
// also returns pointer to "out_image_pointer" and "out_border_pointer" of textures that should be displayed in animation:
    void execute_animation(SDL_Texture** out_image_pointer,SDL_Texture** out_border_pointer,SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps);
//      it's the same function as "execute_animation" except it's additionally draws animation into render;
    void execute_animation_draw(SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps,SDL_Renderer* render);

//      changes "original_images_h" & "original_images_w";
// if u don't want to change some variable,just place there -1;
    void change_images_original_w_h(int32_t original_images_h,int32_t original_images_w);
//      changes "original_borders_h" & "original_borders_w";
// if u don't want to change some variable,just place there -1;
    void change_borders_original_w_h(int32_t original_borders_h,int32_t original_borders_w);

//      adds number to actual scalling variable;
    void change_scaling_dynamically(float scaling);
//      swaps actual scaling variable inside class with this one;
// "scaling" should be above 0.0;
    void change_scaling(float scaling);
    void change_scaling_type(ScalingType scaling_type);

// number should be above 0.0;
    void change_speed(float speed);
    void change_speed_dynamically(float speed);
// if "left_or_right" == 0 then direction will be left; else if 1 then will be right;
    void change_direction(bool left_or_right);

    void change_image_front(SDL_Texture* front_image);
    void change_image_back(SDL_Texture* back_image);
    void change_border_front(SDL_Texture* front_border);
    void change_border_back(SDL_Texture* back_border);

// returns stage of half flips;
//      returns 0 if animation is at first stage;
//      returns 1 if animation is at second stage;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_half_flip_stage() const;

// returns stage of whole flips;
//      returns 0 if animation is at stage where
//          "image_front" is as front at default;
//      returns 1 if animation is at stage where "image_back" is as front at default;
// return variables may be reversed if direction of animation changes cause variable is relative;
    bool get_whole_flip_stage() const;

private:
//      main tool of "executa_animation" & "executa_animation_draw" functions;
    void _main_execute_animation(SDL_Rect* _io_rect_images,SDL_Rect* _io_rect_borders,uint32_t _actual_fps);

// this function is universal and can be used separately without class;
//      3D animation designed to use with SDL_Texture and SDL_Rect;
// "io_main_y" can be 0 when passed to function; outputs new y of texture by adding new value to variable;
// "io_main_h" should have original height in pixels of texture when passed to function; it outputs and overwrites height of texture in variable;
// "io_border_y" can be 0 when passed to function; outputs new y of border texture by adding new value to variable;
// "io_border_h" should have original height in pixels of border texture when passed to function; it outputs and overwrites height of border texture in variable;
//      "io_border_h" should be lower than "io_main_h" otherwise there will be problems;
// "half_flip" tells stage of animation;
// "rotate_y" should be from 0.0 to max 1.0;
void _cSDL_anim3D_vertical_flip_border(int32_t& io_main_y,int32_t& io_main_h,
                                    int32_t& io_border_y,int32_t& io_border_h,bool half_flip,float rotate_y);


};

cSDL_Anim3D_Yaxis_border::cSDL_Anim3D_Yaxis_border(SDL_Texture* front_image,SDL_Texture* back_image,int32_t original_images_h,int32_t original_images_w,
                                SDL_Texture* front_border,SDL_Texture* back_border,int32_t original_borders_h,int32_t original_borders_w,
                                    float speed,AnimationSpeedType speed_type)
{
    this->_front_image = front_image;
    this->_back_image = back_image;
    this->_front_border = front_border;
    this->_back_border = back_border;
    this->_original_images_h = original_images_h;
    this->_original_images_w = original_images_w;
    this->_original_borders_h = original_borders_h;
    this->_original_borders_w = original_borders_w;
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    this->_speed_type = speed_type;

    this->_half_flip = 0;
    this->_whole_flip = 0;
    this->_pointer_image = front_image;
    this->_pointer_border = front_border;
    this->_rotate_y_variable = 0.0;
    this->_direction = 1;

    this->_scaling_type = ScalingType::CENTERED_SCALING;
    this->_scaling = 1.0;

    return;
}

void cSDL_Anim3D_Yaxis_border::execute_animation(SDL_Texture** out_image_pointer,SDL_Texture** out_border_pointer,SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps)
{
    if(this->_scaling_type==ScalingType::RAW_SCALING)
    {
        io_rect_images->h = this->_original_images_h*this->_scaling;
        io_rect_images->w = this->_original_images_w*this->_scaling;

        io_rect_borders->h = this->_original_borders_h*this->_scaling;
        io_rect_borders->w = this->_original_borders_w*this->_scaling;
    }
    else if(this->_scaling_type==ScalingType::CENTERED_SCALING)
    {
        io_rect_images->h = this->_original_images_h*this->_scaling;
        io_rect_images->w = this->_original_images_w*this->_scaling;
        io_rect_borders->h = this->_original_borders_h*this->_scaling;
        io_rect_borders->w = this->_original_borders_w*this->_scaling;

        io_rect_images->x+=(this->_original_images_w-(io_rect_images->w))/2;
        io_rect_images->y+=(this->_original_images_h-(io_rect_images->h))/2;
        io_rect_borders->x+=(this->_original_images_w-(io_rect_images->w))/2;
        io_rect_borders->y+=(this->_original_images_h-(io_rect_images->h))/2;
    }

    this->_main_execute_animation(io_rect_images,io_rect_borders,actual_fps);
    *out_image_pointer = this->_pointer_image;
    *out_border_pointer = this->_pointer_border;


    return;
}

void cSDL_Anim3D_Yaxis_border::execute_animation_draw(SDL_Rect* io_rect_images,SDL_Rect* io_rect_borders,uint32_t actual_fps,SDL_Renderer* render)
{
    SDL_Texture* out_image_pointer;
    SDL_Texture* out_border_pointer;
    this->execute_animation(&out_image_pointer,&out_border_pointer,io_rect_images,io_rect_borders,actual_fps);
    SDL_RenderCopy(render,out_image_pointer,NULL,io_rect_images);
    SDL_RenderCopy(render,out_border_pointer,NULL,io_rect_borders);
    return;
}

void cSDL_Anim3D_Yaxis_border::change_images_original_w_h(int32_t original_images_h,int32_t original_images_w)
{
    if(original_images_h!=-1) this->_original_images_h = original_images_h;
    if(original_images_w!=-1) this->_original_images_w = original_images_w;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_borders_original_w_h(int32_t original_borders_h,int32_t original_borders_w)
{
    if(original_borders_h!=-1) this->_original_borders_h = original_borders_h;
    if(original_borders_w!=-1) this->_original_borders_w = original_borders_w;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_scaling_dynamically(float scaling)
{
    this->_scaling+=scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_scaling(float scaling)
{
    this->_scaling = scaling;
    if(this->_scaling<0.0) this->_scaling = 0.0;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_scaling_type(ScalingType scaling_type)
{
    this->_scaling_type = scaling_type;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_speed(float speed)
{
    if(speed<0.0) this->_speed = 0.0;
    else this->_speed = speed;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_speed_dynamically(float speed)
{
    this->_speed+=speed;
    if(this->_speed<0.0) this->_speed = 0.0;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_direction(bool left_or_right)
{
    this->_direction = left_or_right;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_image_front(SDL_Texture* front_image)
{
    this->_front_image = front_image;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_image_back(SDL_Texture* back_image)
{
    this->_back_image = back_image;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_border_front(SDL_Texture* front_border)
{
    this->_front_border = front_border;
    return;
}

void cSDL_Anim3D_Yaxis_border::change_border_back(SDL_Texture* back_border)
{
    this->_back_border = back_border;
    return;
}

bool cSDL_Anim3D_Yaxis_border::get_half_flip_stage() const
{
    return this->_half_flip;
}

bool cSDL_Anim3D_Yaxis_border::get_whole_flip_stage() const
{
    return this->_whole_flip;
}

void cSDL_Anim3D_Yaxis_border::_main_execute_animation(SDL_Rect* _io_rect_images,SDL_Rect* _io_rect_borders,uint32_t _actual_fps)
{
    if(_actual_fps==0) _actual_fps = 1;

    float rotate_y;

//  i know that code is repeating but it's just easier to follow for me this way;
    if(this->_speed_type==AnimationSpeedType::SPEED_COS)
    {
        if(this->_direction==1)
        {
            this->_rotate_y_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if((this->_rotate_y_variable/M_PI)>1.0)
            {
                this->_rotate_y_variable-=M_PI;
                if(this->_rotate_y_variable>M_PI) this->_rotate_y_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_y_variable/M_PI)>=0.5)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }
        }
        else
        {
            this->_rotate_y_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_y_variable<0.0)
            {
                this->_rotate_y_variable+=M_PI;
                if(this->_rotate_y_variable<0.0) this->_rotate_y_variable = M_PI; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_y_variable/M_PI)<0.5)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }

        rotate_y = fabsf(cosf(this->_rotate_y_variable)); // 1 -> 0 -> 1 (cosf depends on M_PI);
    }
    else
    {
        if(this->_direction==1)
        {
            this->_rotate_y_variable+=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_y_variable>2.0)
            {
                this->_rotate_y_variable-=2.0;
                if(this->_rotate_y_variable>2.0) this->_rotate_y_variable = 0.0; // security check;

                this->_half_flip = 0;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_y_variable)>=1.0)
            {
                this->_half_flip = 1;

                if(this->_whole_flip==0) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        else
        {
            this->_rotate_y_variable-=(1.0/(double)_actual_fps)*this->_speed;
            if(this->_rotate_y_variable<0.0)
            {
                this->_rotate_y_variable+=2.0;
                if(this->_rotate_y_variable<0.0) this->_rotate_y_variable = 2.0; // security check;

                this->_half_flip = 1;
                if(this->_whole_flip==0)
                {
                    this->_whole_flip = 1;
                    this->_pointer_border = this->_back_border;
                }
                else
                {
                    this->_whole_flip = 0;
                    this->_pointer_border = this->_front_border;
                }
            }
            else if((this->_rotate_y_variable)<1.0)
            {
                this->_half_flip = 0;

                if(this->_whole_flip==1) this->_pointer_image = this->_back_image;
                else this->_pointer_image = this->_front_image;
            }

        }
        rotate_y = fabsf(1.0-this->_rotate_y_variable); // 1 -> 0 -> 1 (fabsf converts negative numbers to posiiive);
    }

    this->_cSDL_anim3D_vertical_flip_border(_io_rect_images->y,_io_rect_images->h,_io_rect_borders->y,_io_rect_borders->h,this->_half_flip,rotate_y);

    return;
}

void cSDL_Anim3D_Yaxis_border::_cSDL_anim3D_vertical_flip_border(int32_t& io_main_y,int32_t& io_main_h,
                                    int32_t& io_border_y,int32_t& io_border_h,bool half_flip,float rotate_y)
{
    if(rotate_y>1.0) rotate_y = 1.0;                         // "rotate_y" cannot be larger than 1;
    else if (rotate_y<(float)(1.0/io_main_h)) rotate_y = 0.0;   // just to be sure that "out_h_border" and "out_h" are 0;

    const int32_t out_h_border = (int32_t)(io_border_h-(io_border_h*rotate_y)); // final height of border;

    const int32_t out_h = (int32_t)(io_main_h*rotate_y); // final height of main image;
    const int32_t off_y = (int32_t)(((io_main_h-out_h)+out_h_border)/2);	// position offset of main image;



    // main_image
//==========================================================

    int32_t flip_offset;
    if(half_flip==0)            //  front side of the main image;
    {
        flip_offset = 0;
    }
    else                        //  back side of the main image;
    {
        flip_offset = out_h_border;
    }
    io_main_y+=off_y-flip_offset;
    io_main_h = out_h;


//==========================================================


    // border
//==========================================================

    int32_t off_y_border;
    if(half_flip==0)
    {
        off_y_border = (off_y-out_h_border);  // -- dangerous --

    }
    else off_y_border = ((off_y+out_h)-out_h_border);
    io_border_y+=off_y_border;
    io_border_h = out_h_border;
//==========================================================

    return;
}










//      cSDL class for events; class isn't idiot-resistant;
// needs class "CustomVector";
// #include <SDL2/SDL.h>
// #include <stdint.h>
// #include <stdlib.h>
class cSDL_ButtonsManager
{
public:
    enum FocusType {KEYBOARD,MOUSE};
    enum TypeOfFocusInteraction {NEAREST_LEFT,NEAREST_RIGHT,NEAREST_UP,NEAREST_DOWN,FAR_LEFT,FAR_RIGHT,FAR_DOWN,FAR_UP};
    enum TypeOfPress {DOWN_UP,DOWN,UP,HOLD_DOWN_COMBINATION};
    enum MouseEventValue {MOUSE_BUTTON_RIGHT = SDL_BUTTON_RIGHT,MOUSE_BUTTON_LEFT = SDL_BUTTON_LEFT,
                            MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,MOUSE_BUTTON_X1 = SDL_BUTTON_X1,MOUSE_BUTTON_X2 = SDL_BUTTON_X2};
    enum TypeOfPressToGainFocus {PRESS_DOWN,PRESS_UP,PRESS_DOWN_OR_UP};

    cSDL_ButtonsManager();

//      general functions that are not related to buttons/events:
//---------------------------------------------------------------------------------------------

//      by enabling scaling mechanism,u enabling additional calculation witch variables "focus_interaction_area" & "button_dimensions"; by default it's disabled;
//          after enabling this function u should use function "update_scaling_mechanism()" once at the beginning and after every change of window dimensions;
//  "enable": 1 to enable,0 to disable;
    void enable_scaling_mechanism(bool enable);
//      u should use this function after enbaling scaling mechanism by func "enable_scaling_mechanism()" otherwise this func has no effect;
//          u should run this function once after running "enable_scaling_mechanism()" and everytime when dimension of the window are changed;
    void update_scaling_mechanism(uint32_t oryginal_window_w,uint32_t oryginal_window_h,uint32_t new_window_w,uint32_t new_window_h);

//      force to check if focuse should be changed based from one object to another based on actual mouse position; this function is a little heavy;
//          it's usefull when u're for example pressed left mouse button and at the same new object has been created under mouse cursor
//          and to make focus at that object u have manually by this function check mouse position if cursor is pointing at some object;
    void force_mouse_position_event_update();

//      enable mechanism where func "run_event_checker()" doesn't check mouse motion and only func "force_mouse_position_event_update()" checks;
//          it's usefull when u want to check if ur button get focus every frame and not only per every event of program;
//          u should after enabling this,run "force_mouse_position_event_update()" every frame; by default it's 0 (OFF);
    void enable_check_mouse_motion_only_in_force_mouse_position_event_update(bool enable);

//      forcefully change which device should have focus;
//  "new_focus_device": device type;
    void set_manually_focus_device(cSDL_ButtonsManager::FocusType new_focus_device);

//      forcefully change which button should have focus;
//  "button_id": id of the button of which u want to perform operation on;
    void set_manually_button_focus(uint32_t button_id);

//      forcefully disable focus of any button;
    void disable_manually_button_focus();

//      get actual device that has focus;
    cSDL_ButtonsManager::FocusType get_actual_focus_device();

//      functions to manipulate behaviour of gaining focus of the devices;
//---------------------------
//      these 4 functions has no effect until u enable "all_keys" mode by functions:
//          "enable_mode_all_keys_to_key_focus()" or "enable_mode_all_keys_to_mouse_focus()";
    void add_option_to_key_focus(SDL_Keycode option_to_add);
    void add_option_to_mouse_focus(cSDL_ButtonsManager::MouseEventValue option_to_add);
    void clear_options_to_key_focus();
    void clear_options_to_mouse_focus();
//      functions to enable/disable mode where by pressing any key from keyboard/mouse u gain focus for that device;
//          if this mode is enabled then any change by "add_option_to_key_focus()" or "add_option_to_mouse_focus()" is ignored;
//          by default this mode is enabled;
    void enable_mode_all_keys_to_key_focus(bool enable);
    void enable_mode_all_keys_to_mouse_focus(bool enable);
//      functions to change type of pressing buttons to gain focus; default is "PRESS_DOWN_OR_UP";
    void change_type_of_gaining_focus_mouse(cSDL_ButtonsManager::TypeOfPressToGainFocus type);
    void change_type_of_gaining_focus_key(cSDL_ButtonsManager::TypeOfPressToGainFocus type);

//---------------------------


//---------------------------------------------------------------------------------------------


//      functions that are related to buttons/events:
//---------------------------------------------------------------------------------------------

//      one of the main functions; function to create button;
//  "button_dimensions": dimension and position of the button;
//  "gain_focus_by_mouse_cursor": if 1 then button can gain focus by pointed mouse cursor; if u don't want this feature then pass 0;
//  "user_custom_data": u can pass there anything u want,it doesn't do anything; it can be helped only for u if u want to identify this button somehow;
//  returns "id" of the created object;
    uint32_t create_button(SDL_Rect button_dimensions,bool gain_focus_by_mouse_cursor,uint32_t user_custom_data);
//      one of the main functions; function to create event;
//  "activated_func": should be address of function that will be executed when event activates;
//  "user_data": should be pointer to ur data that u want to pass to the function callback when event activates; can be NULL if u don't need it;
//  returns "id" of the created object;
    uint32_t create_event(void (*activated_func)(void* user_data,SDL_Event* event,cSDL_ButtonsManager* class_obj),void* user_data);

//      function to change callback of existing event;
//  "event_id": id of the event u want to change; if event with this id doesn't exist then function doesn't do anything;
//  "activated_func": should be new pointer to the new function u want to link with the event;
//  "user_data": new user pointer data that u want to be passed to the function callback when event is activated;
    void change_event_callback_function(uint32_t event_id,void (*activated_func)(void* user_data,SDL_Event* event,cSDL_ButtonsManager* class_obj),void* user_data);

//      there are important functions that should be executed after creating event; these functions manipulate which keys should be pressed to activate event;
    void add_required_key_for_event(uint32_t event_id,cSDL_ButtonsManager::TypeOfPress type,SDL_Keycode key);
    void add_required_mouse_key_for_event(uint32_t event_id,cSDL_ButtonsManager::TypeOfPress type,cSDL_ButtonsManager::MouseEventValue mouse_key);
    void add_required_special_type_for_event(uint32_t event_id,SDL_EventType type);
    void delete_all_required_keys_for_event(uint32_t event_id);
    void delete_all_required_mouse_keys_for_event(uint32_t event_id);
    void delete_all_required_special_types_for_event(uint32_t event_id);
//      "delete_all_requires_for_event()" is just combination of 3 functions above;
    void delete_all_requires_for_event(uint32_t event_id);

//      by running this function u enabling mechanism of "auto focus changing" which is by default disabled; it will automatically change focus of button
//          after activating event depending on settings u pass to function "change_event_auto_focus_mechanism()";
//  "focus_interaction_area": decides about area of the screen that buttons to change focus are searched; whole button must fit in this area to be counted;
//  "main_focus_interaction": decides about behaviour of searching mechanism for new button to focus after activating event;
//  "additional_focus_interaction": if "main_focus_interaction" is impossible to be successfull then this variable is important;
//  "max_perpendicular_difference": if interaction is for example left or right then choosen button will be not the one that has larger Y difference of
//      the actual and next button than in variable " max_perpendicular_difference"; it's the same with up or down interaction and X of the button; difference is in pixels;
//  "max_perpendicular_difference_additional": it's the same as "max_perpendicular_difference" except it works only with "additional_focus_interaction";
//  "default_button_focus_id": if there isn't any focused button then after activating event,this button gets focus at the start; pass 0 if u don't want this option;
    void change_event_auto_focus_mechanism(uint32_t event_id,SDL_Rect focus_interaction_area,cSDL_ButtonsManager::TypeOfFocusInteraction main_focus_interaction,
                                           cSDL_ButtonsManager::TypeOfFocusInteraction additional_focus_interaction,uint32_t max_perpendicular_difference,
                                           uint32_t max_perpendicular_difference_additional,uint32_t default_button_focus_id);

//      functions to change button attributes;
//  "button_id": should be id of the button u want to change attribute; if button with this id doesn't exist then function doesn't do anything;
    void change_button_dimensions(uint32_t button_id,SDL_Rect button_dimensions);
    void change_button_attribute_to_gain_focus_by_cursor(uint32_t button_id,bool gain_focus_by_mouse_cursor);
    void change_button_user_custom_data(uint32_t button_id,uint32_t user_custom_data);

//      functions that return number of created buttons/events;
    size_t get_count_of_buttons() const;
    size_t get_count_of_events() const;

//      functions to delete buttons/events;
//  "from_index": decides from what index of vector deleting should start; if it's larger than "to_index" or maximum
//      available index then function fails;
//  "to_index": decides at what index of vector should deleting operation stop; if "to_index" is lower than "from_index"
//      then function fails; if "to_index" is larger than maximum index then "to_index" is changed automatically to maximum index;
//  "id": id of element to delete;
    void delete_buttons(uint32_t from_index,uint32_t to_index);
    void delete_events(uint32_t from_index,uint32_t to_index);
    void delete_buttons(uint32_t id);
    void delete_events(uint32_t id);
    void delete_all_events();
    void delete_all_buttons();

//      get index/id/user_custom_data of actual focused button;
//  returns id/index/user_custom_data; if there's no button that has focus then except "id" func returns 0 and except "index" func returns 0xFFFFFFFF
//      and exctep "user_custom_data" funct returns 0xFFFFFFFF;
    size_t get_focused_button_index() const;
    uint32_t get_focused_button_id() const;
    uint32_t get_focused_button_user_custom_data() const;

//      get indexes of the events/buttons;
//  functions will return 0xFFFFFFFF if there is no button/event with passed id;
    size_t get_button_index(uint32_t id) const;
    size_t get_event_index(uint32_t id) const;

//---------------------------------------------------------------------------------------------


//      main functions:
//---------------------------------------------------------------------------------------------

//      this function is the most heavy; should be called everytime u call "SDL_PollEvent()";
    void run_event_checker(SDL_Event* event);

//---------------------------------------------------------------------------------------------



    struct SpecialEvent
    {
        SDL_EventType type; // the most popular will be like "SDL_MOUSEWHEEL" and "SDL_MOUSEMOTION";
    };
    struct MouseEvent
    {
        TypeOfPress type;
        MouseEventValue mouse_key;

        bool pressing_down;    // this value only will be 1 only when "type" is "DOWN" or "DOWN_UP" and u're pressing and not releasing the button;
        uint32_t last_pressed_focused_button_id;    // important only if "type" is "DOWN_UP";

    };
    struct KeyEvent
    {
        TypeOfPress type;
        SDL_Keycode key;    // like SDLK_a,SDLK_F1,SDLK_RIGHT and etc.;

        bool pressing_down;    // this value only will be 1 only when "type" is "DOWN" or "DOWN_UP" and u're pressing and not releasing the button;
        uint32_t last_pressed_focused_button_id;    // important only if "type" is "DOWN_UP";

    };
    struct GainKeyFocus
    {
        CustomVector<SDL_Keycode> gain_focus_options; // if u hit any of this buttons,device gain focus;
        bool gain_focus_by_any_key_press; //  by default this variable is 1 (ON); if it's 1 then "gain_focus_options" is ignored;
        TypeOfPressToGainFocus type_of_press;   // default is "PRESS_DOWN_OR_UP";
    };
    struct GainMouseFocus
    {
        CustomVector<MouseEventValue> gain_focus_options;   // if u hit any of this buttons,device gain focus;
        bool gain_focus_by_any_mouse_press; //  by default this variable is 1 (ON); if it's 1 then "gain_focus_options" is ignored;
        TypeOfPressToGainFocus type_of_press;   // default is "PRESS_DOWN_OR_UP";
    };

    struct Button
    {
        SDL_Rect button_dimensions; // size and position of the button;

        uint32_t id;    // special unical id for this button; no other button can have this id; shouldn't be changed manually;
        bool gain_focus_by_mouse_cursor; // is this button interactive to mouse cursor in form of gaining focus by placing cursor at that button;

        uint32_t user_custom_data;  // user can define this variable to identify this button; by default it's 0 and variable is not usable by the class mechanisms;
    };

    struct Event
    {
        void (*activated_func)(void* user_data,SDL_Event* event,cSDL_ButtonsManager* class_obj);
        void* user_data; // pointer to data that user passes when he creates this event structure;
        uint32_t id;    // special unical id for this event; no other event can have this id; shouldn't be changed manually;

        CustomVector<KeyEvent> k_events;
        CustomVector<MouseEvent> m_events;
        CustomVector<SpecialEvent> s_events;

        SDL_Rect focus_interaction_area;  // make w == -1 it u don't want to use "auto changing focus" mechanism; by default it's -1 (OFF); this variable decides about
                                            //      size of the area that buttons can be seen; if some button is not as a whole in this area then "auto changing focus" will not be performed on that button;
        TypeOfFocusInteraction main_focus_interaction; // if focus_interaction_area.w == -1 then variable doesn't matter; otherwise it decides about mechanism of changing focus
                                                        //      to other button when all event requirements are met;
        TypeOfFocusInteraction addtional_focus_interaction; // if focus_interaction_area.w == -1 then variable doesn't matter;
                                                            //      when cannot perform "main_focus_interaction" then class will try perform this operation;
        uint32_t max_perpendicular_difference;  // if focus_interaction_area.w == -1 then variable doesn't matter; if interaction is for example left or right
                                                    //  then choosen button will be not the one that has larger Y difference of the actual and next button than in variable " perpendicular_difference";
                                                    //  it's the same with up or down interaction and X of the button; difference is in pixels;
        uint32_t max_perpendicular_difference_additional;   // if focus_interaction_area.w == -1 then variable doesn't matter; it's the same as "max_perpendicular_difference" except it works only
                                                                    //  with "additional_focus_interaction";

        uint32_t default_button_focus_id;   //  if none of the buttons has focus then after activate this event,get focus of this button at first;
                                            //          if id is 0 then there's no default button focus; if id isn't 0 and there's no button with this id then
                                            //          it will take first button of the vector "_button" if there is any;
    };

private:
    CustomVector<Event> _events;
    uint32_t _oryginal_event_id;   // this variable only increasing its value; it's for creating "id" for "Event";

    FocusType _actual_device_focus;     // by default it's "KEYBOARD";
    GainKeyFocus _key_focus_options;
    GainMouseFocus _mouse_focus_options;

    CustomVector<Button> _buttons;
    uint32_t _oryginal_button_id;   // this variable only increasing its value; it's for creating "id" for "Button";
    size_t _actual_button_index_focus;   // if it's 0xFFFFFFFF then there's no focus on any button; by default it's 0xFFFFFFFF; it's for optymalization reasons;
    uint32_t _actual_button_id_focus;     // if it's 0 then there's no focus on any button; by default it's 0;

    bool _check_mouse_motion_only_in_force_mouse_position_event_update; // to enable/disable checking mouse motion from function "run_event_checker()"; by default it's 0 (OFF);
    double _scale_button_dimensions_x; double _scale_button_dimensions_y;
    bool _do_buttons_scals_with_window_w_h; // by setting it '1' ur class will calculates "button_dimensions" that way:
                                                // button_dimensions*_scale_button_dimensions; by default it's '0' (OFF);
                                                // it's mainly for correct mouse position checker;

public:
// manual dangerous functions; use this functions only if u're know what u doing; remember that "CustomVector" variables
//      can change their inside variables address so be carefull; if u want to use these functions safely,u shouldn't change their variables,
//      and while using these vectors,u shouldn't be using any functions that creates/erases buttons/events like "create_button()" or "delete_buttons()";
//-------------------------------------------------------------------------------------------------------------------------

    CustomVector<Event>& get_event_vector();
    CustomVector<Button>& get_button_vector();

//-------------------------------------------------------------------------------------------------------------------------
};

cSDL_ButtonsManager::cSDL_ButtonsManager()
{
    this->_oryginal_event_id = 1;

    this->_actual_device_focus = FocusType::KEYBOARD;
    this->_key_focus_options.gain_focus_by_any_key_press = 1;
    this->_key_focus_options.type_of_press = TypeOfPressToGainFocus::PRESS_DOWN_OR_UP;
    this->_mouse_focus_options.gain_focus_by_any_mouse_press = 1;
    this->_mouse_focus_options.type_of_press = TypeOfPressToGainFocus::PRESS_DOWN_OR_UP;

    this->_oryginal_button_id = 1;
    this->_actual_button_index_focus = 0xFFFFFFFF;
    this->_actual_button_id_focus = 0;

    this->_check_mouse_motion_only_in_force_mouse_position_event_update = 0;
    this->_do_buttons_scals_with_window_w_h = 0;

    return;
}

void cSDL_ButtonsManager::enable_scaling_mechanism(bool enable)
{
    this->_do_buttons_scals_with_window_w_h = enable;
    this->_scale_button_dimensions_x = 1.0;
    this->_scale_button_dimensions_y = 1.0;
    return;
}

void cSDL_ButtonsManager::update_scaling_mechanism(uint32_t oryginal_window_w,uint32_t oryginal_window_h,uint32_t new_window_w,uint32_t new_window_h)
{
    this->_scale_button_dimensions_x = (double)((double)new_window_w/(double)oryginal_window_w);
    this->_scale_button_dimensions_y = (double)((double)new_window_h/(double)oryginal_window_h);
    return;
}

void cSDL_ButtonsManager::force_mouse_position_event_update()
{
    if(this->_actual_button_index_focus!=0xFFFFFFFF&&this->_actual_device_focus==FocusType::MOUSE)
    {
        this->_actual_button_id_focus = 0;
        this->_actual_button_index_focus = 0xFFFFFFFF;
    }

    int32_t mouse_x,mouse_y;
    SDL_GetMouseState(&mouse_x,&mouse_y);

    int64_t count_of_buttons = this->_buttons.size();
    count_of_buttons--;
    for(int64_t i = count_of_buttons; i>=0; i--)
    {
        if(this->_buttons[i].gain_focus_by_mouse_cursor==1)
        {
            SDL_Rect dimensions = this->_buttons[i].button_dimensions;
            if(this->_do_buttons_scals_with_window_w_h==1)
            {
                dimensions.x*=this->_scale_button_dimensions_x;
                dimensions.w*=this->_scale_button_dimensions_x;
                dimensions.y*=this->_scale_button_dimensions_y;
                dimensions.h*=this->_scale_button_dimensions_y;
            }
            if((mouse_x>=dimensions.x&&mouse_x<=(dimensions.x+dimensions.w))&&(mouse_y>=dimensions.y&&mouse_y<=(dimensions.y+dimensions.h)))
            {
                    this->_actual_button_id_focus = this->_buttons[i].id;
                    this->_actual_button_index_focus = i;
                    this->_actual_device_focus = FocusType::MOUSE;
                    break;
            }
        }
    }



    return;
}

void cSDL_ButtonsManager::enable_check_mouse_motion_only_in_force_mouse_position_event_update(bool enable)
{
    this->_check_mouse_motion_only_in_force_mouse_position_event_update = enable;
    return;
}

void cSDL_ButtonsManager::set_manually_focus_device(cSDL_ButtonsManager::FocusType new_focus_device)
{
    this->_actual_device_focus = new_focus_device;
    return;
}

void cSDL_ButtonsManager::set_manually_button_focus(uint32_t button_id)
{
    size_t size = this->_buttons.size();
    for(size_t i = 0; i<size; i++)
    {
        if(button_id==this->_buttons[i].id)
        {
            this->_actual_button_id_focus = button_id;
            this->_actual_button_index_focus = i;
            break;
        }
    }
    return;
}

void cSDL_ButtonsManager::disable_manually_button_focus()
{
    this->_actual_button_id_focus = 0;
    this->_actual_button_index_focus = 0xFFFFFFFF;
    return;
}

cSDL_ButtonsManager::FocusType cSDL_ButtonsManager::get_actual_focus_device()
{
    return this->_actual_device_focus;
}

void cSDL_ButtonsManager::add_option_to_key_focus(SDL_Keycode option_to_add)
{
    this->_key_focus_options.gain_focus_options.push_back(option_to_add);
    return;
}
void cSDL_ButtonsManager::add_option_to_mouse_focus(cSDL_ButtonsManager::MouseEventValue option_to_add)
{
    this->_mouse_focus_options.gain_focus_options.push_back(option_to_add);
    return;
}
void cSDL_ButtonsManager::clear_options_to_key_focus()
{
    this->_key_focus_options.gain_focus_options.clear();
    return;
}
void cSDL_ButtonsManager::clear_options_to_mouse_focus()
{
    this->_mouse_focus_options.gain_focus_options.clear();
    return;
}

void cSDL_ButtonsManager::enable_mode_all_keys_to_key_focus(bool enable)
{
    this->_key_focus_options.gain_focus_by_any_key_press = enable;
    return;
}
void cSDL_ButtonsManager::enable_mode_all_keys_to_mouse_focus(bool enable)
{
    this->_mouse_focus_options.gain_focus_by_any_mouse_press = enable;
    return;
}

void cSDL_ButtonsManager::change_type_of_gaining_focus_mouse(cSDL_ButtonsManager::TypeOfPressToGainFocus type)
{
    this->_mouse_focus_options.type_of_press = type;
    return;
}
void cSDL_ButtonsManager::change_type_of_gaining_focus_key(cSDL_ButtonsManager::TypeOfPressToGainFocus type)
{
    this->_key_focus_options.type_of_press = type;
    return;
}

uint32_t cSDL_ButtonsManager::create_button(SDL_Rect button_dimensions,bool gain_focus_by_mouse_cursor,uint32_t user_custom_data)
{
    uint32_t button_id = this->_oryginal_button_id;
    this->_oryginal_button_id++;

    Button button;
    button.button_dimensions = button_dimensions;
    button.gain_focus_by_mouse_cursor = gain_focus_by_mouse_cursor;
    button.id = button_id;
    button.user_custom_data = user_custom_data;
    this->_buttons.push_back(button);

    size_t size = this->_buttons.size();
    bool should_be_actual_focus_disabled = 1;
    for(size_t i = 0; i<size; i++)
    {
        if(this->_buttons[i].id==this->_actual_button_id_focus)
        {
            this->_actual_button_index_focus = i;
            should_be_actual_focus_disabled = 0;
            break;
        }
    }
    if(should_be_actual_focus_disabled==1)
    {
        this->_actual_button_id_focus = 0;
        this->_actual_button_index_focus = 0xFFFFFFFF;
    }

    return button_id;
}

uint32_t cSDL_ButtonsManager::create_event(void (*activated_func)(void* user_data,SDL_Event* event,cSDL_ButtonsManager* class_obj),void* user_data)
{
    uint32_t event_id = this->_oryginal_event_id;
    this->_oryginal_event_id++;

    Event event;
    event.activated_func = activated_func;
    event.focus_interaction_area.w = -1;        // -1 to OFF "auto focus changing" mechanism;
    event.default_button_focus_id = 0;
    event.id = event_id;
    event.user_data = user_data;

    this->_events.push_back(event);

    return event_id;
}

void cSDL_ButtonsManager::change_event_callback_function(uint32_t event_id,void (*activated_func)(void* user_data,SDL_Event* event,cSDL_ButtonsManager* class_obj),void* user_data)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_events[i].activated_func = activated_func;
    this->_events[i].user_data = user_data;

    return;
}

void cSDL_ButtonsManager::add_required_key_for_event(uint32_t event_id,cSDL_ButtonsManager::TypeOfPress type,SDL_Keycode key)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    KeyEvent key_event;
    key_event.key = key;
    key_event.pressing_down = 0;
    key_event.last_pressed_focused_button_id = 0;
    key_event.type = type;

    this->_events[i].k_events.push_back(key_event);

    return;
}
void cSDL_ButtonsManager::add_required_mouse_key_for_event(uint32_t event_id,cSDL_ButtonsManager::TypeOfPress type,cSDL_ButtonsManager::MouseEventValue mouse_key)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    MouseEvent mouse_event;
    mouse_event.mouse_key = mouse_key;
    mouse_event.pressing_down = 0;
    mouse_event.last_pressed_focused_button_id = 0;
    mouse_event.type = type;

    this->_events[i].m_events.push_back(mouse_event);

    return;
}
void cSDL_ButtonsManager::add_required_special_type_for_event(uint32_t event_id,SDL_EventType type)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    SpecialEvent special_event;
    special_event.type = type;

    this->_events[i].s_events.push_back(special_event);

    return;
}
void cSDL_ButtonsManager::delete_all_required_keys_for_event(uint32_t event_id)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_events[i].k_events.clear();

    return;
}
void cSDL_ButtonsManager::delete_all_required_mouse_keys_for_event(uint32_t event_id)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_events[i].m_events.clear();

    return;
}
void cSDL_ButtonsManager::delete_all_required_special_types_for_event(uint32_t event_id)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_events[i].s_events.clear();

    return;
}
void cSDL_ButtonsManager::delete_all_requires_for_event(uint32_t event_id)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_events[i].k_events.clear();
    this->_events[i].m_events.clear();
    this->_events[i].s_events.clear();

    return;
}

void cSDL_ButtonsManager::change_event_auto_focus_mechanism(uint32_t event_id,SDL_Rect focus_interaction_area,cSDL_ButtonsManager::TypeOfFocusInteraction main_focus_interaction,
                                       cSDL_ButtonsManager::TypeOfFocusInteraction additional_focus_interaction,uint32_t max_perpendicular_difference,
                                       uint32_t max_perpendicular_difference_additional,uint32_t default_button_focus_id)
{
    bool does_id_exists = 0;
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==event_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_events[i].focus_interaction_area = focus_interaction_area;
    this->_events[i].main_focus_interaction = main_focus_interaction;
    this->_events[i].addtional_focus_interaction = additional_focus_interaction;
    this->_events[i].max_perpendicular_difference = max_perpendicular_difference;
    this->_events[i].max_perpendicular_difference_additional = max_perpendicular_difference_additional;
    this->_events[i].default_button_focus_id = default_button_focus_id;

    return;
}

void cSDL_ButtonsManager::change_button_dimensions(uint32_t button_id,SDL_Rect button_dimensions)
{
    bool does_id_exists = 0;
    size_t size = this->_buttons.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_buttons[i].id==button_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_buttons[i].button_dimensions = button_dimensions;

    return;
}
void cSDL_ButtonsManager::change_button_attribute_to_gain_focus_by_cursor(uint32_t button_id,bool gain_focus_by_mouse_cursor)
{
    bool does_id_exists = 0;
    size_t size = this->_buttons.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_buttons[i].id==button_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_buttons[i].gain_focus_by_mouse_cursor = gain_focus_by_mouse_cursor;

    return;
}
void cSDL_ButtonsManager::change_button_user_custom_data(uint32_t button_id,uint32_t user_custom_data)
{
    bool does_id_exists = 0;
    size_t size = this->_buttons.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_buttons[i].id==button_id)
        {
            does_id_exists = 1;
            break;
        }
    }
    if(does_id_exists==0) return;

    this->_buttons[i].user_custom_data = user_custom_data;

    return;
}

size_t cSDL_ButtonsManager::get_count_of_buttons() const
{
    return this->_buttons.size();
}
size_t cSDL_ButtonsManager::get_count_of_events() const
{
    return this->_events.size();
}

void cSDL_ButtonsManager::delete_buttons(uint32_t from_index,uint32_t to_index)
{
    this->_buttons.erase(from_index,to_index);

    if(this->_actual_button_index_focus!=0xFFFFFFFF)
    {
        bool does_id_exists = 0;
        size_t size = this->_buttons.size();
        size_t i = 0;
        for(; i<size; i++)
        {
            if(this->_buttons[i].id==this->_actual_button_id_focus)
            {
                does_id_exists = 1;
                this->_actual_button_index_focus = i;
                break;
            }
        }
        if(does_id_exists==0)
        {
            this->_actual_button_id_focus = 0;
            this->_actual_button_index_focus = 0xFFFFFFFF;
        }
    }

    return;
}
void cSDL_ButtonsManager::delete_events(uint32_t from_index,uint32_t to_index)
{
    this->_events.erase(from_index,to_index);
    return;
}
void cSDL_ButtonsManager::delete_buttons(uint32_t id)
{
    bool does_id_exists1 = 0;
    size_t size1 = this->_buttons.size();
    for(size_t i = 0; i<size1; i++)
    {
        if(this->_buttons[i].id==id)
        {
            does_id_exists1 = 1;
            this->_buttons.erase(i,i);
            break;
        }
    }
    if(does_id_exists1==0) return;

    if(this->_actual_button_index_focus!=0xFFFFFFFF)
    {
        bool does_id_exists = 0;
        size_t size = this->_buttons.size();
        size_t i = 0;
        for(; i<size; i++)
        {
            if(this->_buttons[i].id==this->_actual_button_id_focus)
            {
                does_id_exists = 1;
                this->_actual_button_index_focus = i;
                break;
            }
        }
        if(does_id_exists==0)
        {
            this->_actual_button_id_focus = 0;
            this->_actual_button_index_focus = 0xFFFFFFFF;
        }
    }
    return;
}
void cSDL_ButtonsManager::delete_events(uint32_t id)
{
    size_t size = this->_events.size();
    size_t i = 0;
    for(; i<size; i++)
    {
        if(this->_events[i].id==id)
        {
            this->_events.erase(i,i);
            break;
        }
    }

    return;
}
void cSDL_ButtonsManager::delete_all_events()
{
    this->_events.clear();
    return;
}
void cSDL_ButtonsManager::delete_all_buttons()
{
    this->_buttons.clear();
    this->_actual_button_id_focus = 0;
    this->_actual_button_index_focus = 0xFFFFFFFF;
    return;
}

size_t cSDL_ButtonsManager::get_focused_button_index() const
{
    return this->_actual_button_index_focus;
}
uint32_t cSDL_ButtonsManager::get_focused_button_id() const
{
    return this->_actual_button_id_focus;
}

uint32_t cSDL_ButtonsManager::get_focused_button_user_custom_data() const
{
    if(this->_actual_button_index_focus==0xFFFFFFFF) return 0xFFFFFFFF;
    else return this->_buttons[this->_actual_button_index_focus].user_custom_data;
}

size_t cSDL_ButtonsManager::get_button_index(uint32_t id) const
{
    size_t returned_index = 0xFFFFFFFF;
    size_t size = this->_buttons.size();

    for(size_t i = 0; i<size; i++)
    {
        if(this->_buttons[i].id==id)
        {
            returned_index = i;
            break;
        }
    }

    return returned_index;
}

size_t cSDL_ButtonsManager::get_event_index(uint32_t id) const
{
    size_t returned_index = 0xFFFFFFFF;
    size_t size = this->_events.size();

    for(size_t i = 0; i<size; i++)
    {
        if(this->_events[i].id==id)
        {
            returned_index = i;
            break;
        }
    }

    return returned_index;
}

void cSDL_ButtonsManager::run_event_checker(SDL_Event* event)
{
    bool check_mouse_position = 0;
    bool update_forcefully_mouse_position = 0;
    if(this->_check_mouse_motion_only_in_force_mouse_position_event_update==0) check_mouse_position = 1;

//      check which device should have focus;
    if(this->_actual_device_focus!=FocusType::KEYBOARD)
    {
        if(this->_key_focus_options.gain_focus_by_any_key_press==1)
        {
            if(event->type==SDL_KEYDOWN)
            {
                if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN) this->_actual_device_focus = FocusType::KEYBOARD;
                else if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) this->_actual_device_focus = FocusType::KEYBOARD;
            }
            else if(event->type==SDL_KEYUP)
            {
                if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_UP) this->_actual_device_focus = FocusType::KEYBOARD;
                else if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) this->_actual_device_focus = FocusType::KEYBOARD;
            }
        }
        else
        {
            bool try_gaining_focus = 0;
            if(event->type==SDL_KEYDOWN)
            {
                if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN) try_gaining_focus = 1;
                else if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) try_gaining_focus = 1;
            }
            else if(event->type==SDL_KEYUP)
            {
                if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_UP) try_gaining_focus = 1;
                else if(this->_key_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) try_gaining_focus = 1;
            }

            if(try_gaining_focus==1)
            {
                size_t size_of_vector = this->_key_focus_options.gain_focus_options.size();
                for(size_t i = 0; i<size_of_vector; i++)
                {
                    if(this->_key_focus_options.gain_focus_options[i]==event->key.keysym.sym)
                    {
                        this->_actual_device_focus = FocusType::KEYBOARD;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        if(this->_mouse_focus_options.gain_focus_by_any_mouse_press==1)
        {
            bool gain_focus = 0;
            if(event->type==SDL_MOUSEBUTTONDOWN)
            {
                if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN) gain_focus = 1;
                else if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) gain_focus = 1;
            }
            else if(event->type==SDL_MOUSEBUTTONUP)
            {
                if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_UP) gain_focus = 1;
                else if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) gain_focus = 1;
            }
            if(gain_focus==1)
            {
                if(this->_actual_device_focus==FocusType::KEYBOARD) {update_forcefully_mouse_position = 1; check_mouse_position = 1;}
                this->_actual_device_focus = FocusType::MOUSE;
            }
        }
        else
        {
            bool try_gaining_focus = 0;
            if(event->type==SDL_MOUSEBUTTONDOWN)
            {
                if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN) try_gaining_focus = 1;
                else if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) try_gaining_focus = 1;
            }
            else if(event->type==SDL_MOUSEBUTTONUP)
            {
                if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_UP) try_gaining_focus = 1;
                else if(this->_mouse_focus_options.type_of_press==TypeOfPressToGainFocus::PRESS_DOWN_OR_UP) try_gaining_focus = 1;
            }

            if(try_gaining_focus==1)
            {
                size_t size_of_vector = this->_mouse_focus_options.gain_focus_options.size();
                for(size_t i = 0; i<size_of_vector; i++)
                {
                    if(this->_mouse_focus_options.gain_focus_options[i]==event->button.button)
                    {
                        if(this->_actual_device_focus==FocusType::KEYBOARD) {update_forcefully_mouse_position = 1; check_mouse_position = 1;}
                        this->_actual_device_focus = FocusType::MOUSE;
                        break;
                    }
                }
            }
        }
    }



//      check here mouse motion if it captures focus by moving;
    if(check_mouse_position==1)
    {
        int32_t mouse_x,mouse_y;
        bool check_mouse = 0;
        if(update_forcefully_mouse_position==1)
        {
            // set mouse_x and mouse_y through SDL function SDL_GetMouseState;
            SDL_GetMouseState(&mouse_x,&mouse_y);
            check_mouse = 1;
        }
        else
        {
            // set mouse_x and mouse_y through "event";
            if(event->type==SDL_MOUSEMOTION)
            {
                mouse_x = event->motion.x;
                mouse_y = event->motion.y;
                check_mouse = 1;
            }
        }

        if(check_mouse==1)
        {
            if(this->_actual_button_index_focus!=0xFFFFFFFF&&this->_actual_device_focus==FocusType::MOUSE)
            {
                this->_actual_button_id_focus = 0;
                this->_actual_button_index_focus = 0xFFFFFFFF;
            }

            int64_t count_of_buttons = this->_buttons.size();
            count_of_buttons--;
            for(int64_t i = count_of_buttons; i>=0; i--)
            {
                if(this->_buttons[i].gain_focus_by_mouse_cursor==1)
                {
                    SDL_Rect dimensions = this->_buttons[i].button_dimensions;
                    if(this->_do_buttons_scals_with_window_w_h==1)
                    {
                        dimensions.x*=this->_scale_button_dimensions_x;
                        dimensions.w*=this->_scale_button_dimensions_x;
                        dimensions.y*=this->_scale_button_dimensions_y;
                        dimensions.h*=this->_scale_button_dimensions_y;
                    }
                    if((mouse_x>=dimensions.x&&mouse_x<=(dimensions.x+dimensions.w))&&(mouse_y>=dimensions.y&&mouse_y<=(dimensions.y+dimensions.h)))
                    {
                        this->_actual_button_id_focus = this->_buttons[i].id;
                        this->_actual_button_index_focus = i;
                        this->_actual_device_focus = FocusType::MOUSE;
                        break;
                    }
                }
            }
        }

    }


    size_t event_count = this->_events.size();
    for(size_t i = 0; i<event_count; i++)
    {
        const size_t k_size = this->_events[i].k_events.size();
        const size_t m_size = this->_events[i].m_events.size();
        const size_t s_size = this->_events[i].s_events.size();
        size_t parts_of_event_to_complete = 0;

        for(size_t j = 0; j<k_size; j++)
        {
            if(event->key.keysym.sym==this->_events[i].k_events[j].key)
            {
                if(event->type==SDL_KEYDOWN)
                {
                    if((this->_events[i].k_events[j].type==TypeOfPress::DOWN||this->_events[i].k_events[j].type==TypeOfPress::HOLD_DOWN_COMBINATION)&&event->key.repeat==0)
                    {
                        // set that this part of event is completed;
                        parts_of_event_to_complete++;
                    }

                    this->_events[i].k_events[j].last_pressed_focused_button_id = this->_actual_button_id_focus;    // important only if u choose "DOWN_UP" of "this->_events[i].k_events[j].type"
                    this->_events[i].k_events[j].pressing_down = 1;
                }
                else if(event->type==SDL_KEYUP)
                {
                    if(this->_events[i].k_events[j].type==TypeOfPress::UP)
                    {
                        // set that this part of event is completed;
                        parts_of_event_to_complete++;
                    }
                    else if(this->_events[i].k_events[j].type==TypeOfPress::DOWN_UP)
                    {
                        if(this->_events[i].k_events[j].pressing_down==1&&this->_events[i].k_events[j].last_pressed_focused_button_id==this->_actual_button_id_focus)
                        {
                            // set that this part of event is completed;
                            parts_of_event_to_complete++;
                        }
                        this->_events[i].k_events[j].last_pressed_focused_button_id = 0;    // important only if u choose "DOWN_UP" of "this->_events[i].k_events[j].type"
                    }
                    this->_events[i].k_events[j].pressing_down = 0;
                }
            }
            else if(this->_events[i].k_events[j].type==TypeOfPress::HOLD_DOWN_COMBINATION&&this->_events[i].k_events[j].pressing_down==1)
            {
                parts_of_event_to_complete++;
            }
        }

        for(size_t j = 0; j<m_size; j++)
        {
            if(event->button.button==this->_events[i].m_events[j].mouse_key)
            {
                if(event->type==SDL_MOUSEBUTTONDOWN)
                {
                    if(this->_events[i].m_events[j].type==TypeOfPress::DOWN||this->_events[i].m_events[j].type==TypeOfPress::HOLD_DOWN_COMBINATION)
                    {
                        // set that this part of event is completed;
                        parts_of_event_to_complete++;
                    }

                    this->_events[i].m_events[j].last_pressed_focused_button_id = this->_actual_button_id_focus; // important only if u choose "DOWN_UP" of "this->_events[i].m_events[j].type"
                    this->_events[i].m_events[j].pressing_down = 1;
                }
                else if(event->type==SDL_MOUSEBUTTONUP)
                {
                    if(this->_events[i].m_events[j].type==TypeOfPress::UP)
                    {
                        // set that this part of event is completed;
                        parts_of_event_to_complete++;
                    }
                    else if(this->_events[i].m_events[j].type==TypeOfPress::DOWN_UP)
                    {
                        if(this->_events[i].m_events[j].pressing_down==1&&this->_events[i].m_events[j].last_pressed_focused_button_id==this->_actual_button_id_focus)
                        {
                            // set that this part of event is completed;
                            parts_of_event_to_complete++;
                        }
                        this->_events[i].m_events[j].last_pressed_focused_button_id = 0;    // important only if u choose "DOWN_UP" of "this->_events[i].m_events[j].type"
                    }
                    this->_events[i].m_events[j].pressing_down = 0;
                }
            }
            else if(this->_events[i].m_events[j].type==TypeOfPress::HOLD_DOWN_COMBINATION&&this->_events[i].m_events[j].pressing_down==1)
            {
                parts_of_event_to_complete++;
            }
        }

        for(size_t j = 0; j<s_size; j++)
        {
            if(event->type==this->_events[i].s_events[j].type)
            {
                // set that this part of event is complete;
                parts_of_event_to_complete++;
            }
        }

        if(parts_of_event_to_complete==(k_size+m_size+s_size))      // if user defined function should be executed and check if u should make here "auto focus changing" mechanism;
        {
            if(this->_events[i].default_button_focus_id!=0&&this->_actual_button_id_focus==0)   // if default button option is ON;
            {
                bool id_doesnt_exists = 1;
                size_t count_of_buttons = this->_buttons.size();
                for(size_t j = 0; j<count_of_buttons; j++)
                {
                    if(this->_buttons[j].id==this->_events[i].default_button_focus_id)
                    {
                        this->_actual_button_id_focus = this->_buttons[j].id;
                        this->_actual_button_index_focus = j;
                        id_doesnt_exists = 0;
                        break;
                    }
                }
                if(id_doesnt_exists==1)
                {
                    if(count_of_buttons!=0)
                    {
                        this->_actual_button_id_focus = this->_buttons[0].id;
                        this->_actual_button_index_focus = 0;
                    }
                    else
                    {
                        this->_actual_button_id_focus = 0;
                        this->_actual_button_index_focus = 0xFFFFFFFF;
                    }
                }
            }
            else if(this->_events[i].focus_interaction_area.w!=-1&&this->_actual_button_index_focus!=0xFFFFFFFF)  // if "auto focus changing" mechanism is ON;
            {
                SDL_Rect area_searching = this->_events[i].focus_interaction_area;
                if(this->_do_buttons_scals_with_window_w_h==1)
                {
                    area_searching.x*=this->_scale_button_dimensions_x;
                    area_searching.w*=this->_scale_button_dimensions_x;
                    area_searching.y*=this->_scale_button_dimensions_y;
                    area_searching.h*=this->_scale_button_dimensions_y;
                }

                TypeOfFocusInteraction interaction = this->_events[i].main_focus_interaction;
                size_t count_of_buttons = this->_buttons.size();
                int64_t nearest_button_index = -1;
                SDL_Rect nearest_dimensions;
                nearest_dimensions.w = -1;
                uint32_t max_perpendicular_difference = this->_events[i].max_perpendicular_difference;
                for(int32_t iteration = 0; iteration<2; iteration++)
                {

                    switch(interaction)
                    {
                        case TypeOfFocusInteraction::NEAREST_LEFT:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y);
                                    const int32_t new_x = this->_buttons[j].button_dimensions.x;

                                    if((new_x<this->_buttons[this->_actual_button_index_focus].button_dimensions.x)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x>nearest_dimensions.x)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x==nearest_dimensions.x&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }

                        }break;

                        case TypeOfFocusInteraction::NEAREST_RIGHT:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y);
                                    const int32_t new_x = this->_buttons[j].button_dimensions.x;

                                    if((new_x>this->_buttons[this->_actual_button_index_focus].button_dimensions.x)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x<nearest_dimensions.x)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x==nearest_dimensions.x&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }
                        }break;

                        case TypeOfFocusInteraction::NEAREST_UP:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x);
                                    const int32_t new_y = this->_buttons[j].button_dimensions.y;

                                    if((new_y<this->_buttons[this->_actual_button_index_focus].button_dimensions.y)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y>nearest_dimensions.y)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y==nearest_dimensions.y&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }
                        }break;

                        case TypeOfFocusInteraction::NEAREST_DOWN:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x);
                                    const int32_t new_y = this->_buttons[j].button_dimensions.y;

                                    if((new_y>this->_buttons[this->_actual_button_index_focus].button_dimensions.y)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y<nearest_dimensions.y)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y==nearest_dimensions.y&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }
                        }break;

                        case TypeOfFocusInteraction::FAR_LEFT:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y);
                                    const int32_t new_x = this->_buttons[j].button_dimensions.x;

                                    if((new_x<this->_buttons[this->_actual_button_index_focus].button_dimensions.x)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x<nearest_dimensions.x)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x==nearest_dimensions.x&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }
                        }break;

                        case TypeOfFocusInteraction::FAR_RIGHT:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y);
                                    const int32_t new_x = this->_buttons[j].button_dimensions.x;

                                    if((new_x>this->_buttons[this->_actual_button_index_focus].button_dimensions.x)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x>nearest_dimensions.x)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_x==nearest_dimensions.x&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.y-this->_buttons[this->_actual_button_index_focus].button_dimensions.y))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }
                        }break;

                        case TypeOfFocusInteraction::FAR_UP:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x);
                                    const int32_t new_y = this->_buttons[j].button_dimensions.y;

                                    if((new_y<this->_buttons[this->_actual_button_index_focus].button_dimensions.y)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y<nearest_dimensions.y)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y==nearest_dimensions.y&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }
                        }break;

                        case TypeOfFocusInteraction::FAR_DOWN:
                        {
                            for(size_t j = 0; j<count_of_buttons; j++)
                            {
                                if(this->_actual_button_index_focus!=j)
                                {
                                    const uint32_t abs_perpendicular_difference = abs(this->_buttons[j].button_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x);
                                    const int32_t new_y = this->_buttons[j].button_dimensions.y;

                                    if((new_y>this->_buttons[this->_actual_button_index_focus].button_dimensions.y)
                                       &&(abs_perpendicular_difference<=max_perpendicular_difference))
                                    {
                                        if(nearest_dimensions.w==-1)    // first button checking;
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y>nearest_dimensions.y)
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                        else if(new_y==nearest_dimensions.y&&abs_perpendicular_difference<(uint32_t)abs(nearest_dimensions.x-this->_buttons[this->_actual_button_index_focus].button_dimensions.x))
                                        {
                                            nearest_dimensions = this->_buttons[j].button_dimensions;
                                            nearest_button_index = j;
                                        }
                                    }
                                }
                            }
                        }break;

                    }
                    if(iteration==0&&nearest_button_index==-1)
                    {
                        interaction = this->_events[i].addtional_focus_interaction;
                        max_perpendicular_difference = this->_events[i].max_perpendicular_difference_additional;
                        continue;
                    }
                    else break;
                }

                if(nearest_button_index!=-1)
                {
                    this->_actual_button_index_focus = nearest_button_index;
                    this->_actual_button_id_focus = this->_buttons[nearest_button_index].id;
                }

            }

            this->_events[i].activated_func(this->_events[i].user_data,event,this);
            break;
        }
    }

    return;
}

CustomVector<cSDL_ButtonsManager::Event>& cSDL_ButtonsManager::get_event_vector()
{
    return this->_events;
}

CustomVector<cSDL_ButtonsManager::Button>& cSDL_ButtonsManager::get_button_vector()
{
    return this->_buttons;
}


//      cSDL class for manual slider; class isn't fully idiot-resistant;
// needs class "CustomVector";
// #include <SDL2/SDL.h>
// #include <stdint.h>
// #include <stdlib.h>
class cSDL_ManualSlider
{
public:
    enum MouseValues {MOUSE_BUTTON_RIGHT = SDL_BUTTON_RIGHT,MOUSE_BUTTON_LEFT = SDL_BUTTON_LEFT,
                            MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,MOUSE_BUTTON_X1 = SDL_BUTTON_X1,MOUSE_BUTTON_X2 = SDL_BUTTON_X2};
//      constructor;
//  "positions": they are points of position shift where slider can move; typically W and H are just width and height of slider image
//      and X and Y are actual points where slider can move;
    cSDL_ManualSlider(const CustomVector<SDL_Rect>& positions);

//      change possible positions of the slider that u pass to the constructor at first;
//          after calling this functions,variable "actual_position_index" will be changed to 0 and if u want
//          to update this variable then call after this "change_actual_slider_position_index()";
//  "positions": points of position shift where slider can move; typically W and H are just width and height of slider image
//      and X and Y are actual points where slider can move;
    void change_available_slider_positions(const CustomVector<SDL_Rect>& positions);
//      change manually actual position of the slider;
//  "positions_index": is actual index of position that u pass trough "change_positions()" or in constructor at the start;
//      if this variable is larger than available then it will be changed to larger available index;
    void change_actual_slider_position_index(uint32_t positions_index);
//      increase/decrease of how much u want to manually shift slider from actual position;
//  "add_to_index"&"subract_from_index": how much u want to subtract/add to/from actual index position;
//      if u pass vailable too small or too big then function will automatically set actual index to the lowest possible/highest possible;
    void increase_actual_slider_position_index(uint32_t add_to_index);
    void decrease_actual_slider_position_index(uint32_t subtract_from_index);

//      adds event to actual events that can interact with slider; by default there is 1 button that can interact with slider and that is "MOUSE_BUTTON_LEFT";
//  "mouse_event": button that u want to add to a pool of interactive buttons with a slider;
    void add_interactive_button_with_slider(cSDL_ManualSlider::MouseValues mouse_event);
//      clear all of the buttons of the pool that can interact with the slider;
    void clear_interactive_buttons_with_slider();

//      to enable/disable interaction with hitboxes of the line; by default it's 1 (ON);
    void enable_slider_line_interaction_(bool enable);

//      to enable scaling mechanism for position variables u pass trough "change_positions()" or in constructor at the start;
//          by enabling this mechanism by this function,u can use function "update_scaling_mechanism()";
//          by default it's 0 (OFF); scaling variable resets after running this function so u have to run "update_scaling_mechanism()" after calling this func;
//  "enable": 1 to enable,0 to disable;
    void enable_mouse_position_scaling_mechanism(bool enable);
//      to update scaling mechanism;
//  "original_window_w"&"original_window_h": ur original window dimensions;
//  "new_window_w"&"new_window_h": ur new window dimensions;
    void update_scaling_mechanism(uint32_t original_window_w,uint32_t original_window_h,uint32_t new_window_w,uint32_t new_window_h);

//      get actual rect of slider; should be called after "run_checker()";
//  returns rect of slider;
    SDL_Rect get_slider_position() const;
//      get actual slider position index; should be called after "run_cheker()";
//  returns index to position u pass trough "change_actual_position_index()" or in the constructor at the start;
    uint32_t get_slider_position_index() const;
//      get actual count of indexes of slider positions; should be called after "run_cheker()";
//  returns number of how much positions slider has; u pass vector that tells it trough "change_actual_position_index()" or in the constructor at the start;
    size_t get_slider_position_index_count() const;
//      get actual state of slider focus; should be called after "run_checker()";
//  returns state of slider focus; returns 0 if there is no focus,returns 1 if there is focus;
    bool get_slider_focus() const;
//      get actual state of slider held; should be called after "run_checker()";
//  returns 0 if slider isn't held; returns 1 if slider is held;
    bool get_slider_held_state() const;
//      get information if mouse button has been release; everytime this function is called,everytime variable about release state is reseted;
//  returns 1 if release state has been confirmed; returns 0 if there was not any new release states;
    bool get_slider_release_state();



//      main function; should be called everytime u calls "SDL_PollEvent()";
//  "event": updated event variable by function "SDL_PollEvent()";
    void run_checker(SDL_Event* event);


private:
    CustomVector<SDL_Rect> _positions;  // remember to check if this variable size() isn't 0;
    CustomVector<MouseValues> _interactive_slider_buttons;  // by default there's only "MOUSE_BUTTON_LEFT"; remember to check if this variable size() isn't 0;
    bool _slider_focus; // does slider has mouse focsu;
    bool _slider_held;  // does slider keep track of the mouse/is moving by the mouse;
    uint32_t _actual_slider_position_index;    // index of "_positions" that is active right now;
    bool _enable_scaling_mechanism;     // by default it's 0 (OFF);
    bool _button_has_been_released;     // info about release state of mouse button;
    bool _enable_line_interaction;      //  by default it's 1 (ON);

    double _scaling_variable_x;         // it's only matters when "_enable_scaling_mechanism" is 1; it's a calculation variable;
    double _scaling_variable_y;         // it's only matters when "_enable_scaling_mechanism" is 1; it's a calculation variable;
    MouseValues _pressed_down_button;   // it's a calculation variable;
};

cSDL_ManualSlider::cSDL_ManualSlider(const CustomVector<SDL_Rect>& positions)
{
    this->_positions = positions;
    this->_interactive_slider_buttons.push_back(MouseValues::MOUSE_BUTTON_LEFT);
    this->_slider_focus = 0;
    this->_slider_held = 0;
    this->_actual_slider_position_index = 0;
    this->_enable_scaling_mechanism = 0;
    this->_button_has_been_released = 0;
    this->_enable_line_interaction = 1;
    return;
}

void cSDL_ManualSlider::change_available_slider_positions(const CustomVector<SDL_Rect>& positions)
{
    this->_positions = positions;
    this->_actual_slider_position_index = 0;
    return;
}

void cSDL_ManualSlider::change_actual_slider_position_index(uint32_t positions_index)
{
    if(this->_positions.size()==0)
    {
        this->_actual_slider_position_index = 0;
    }
    else if(positions_index>=this->_positions.size())
    {
        this->_actual_slider_position_index = this->_positions.size()-1;
    }
    else this->_actual_slider_position_index = positions_index;

    return;
}

void cSDL_ManualSlider::increase_actual_slider_position_index(uint32_t add_to_index)
{
    uint32_t new_index = this->_actual_slider_position_index+add_to_index;
    if(new_index>=this->_positions.size()) this->_actual_slider_position_index = this->_positions.size()-1;
    else this->_actual_slider_position_index = new_index;
    return;
}

void cSDL_ManualSlider::decrease_actual_slider_position_index(uint32_t subtract_from_index)
{
    if(subtract_from_index>this->_actual_slider_position_index) this->_actual_slider_position_index = 0;
    else this->_actual_slider_position_index-=subtract_from_index;
    return;
}

void cSDL_ManualSlider::add_interactive_button_with_slider(cSDL_ManualSlider::MouseValues mouse_event)
{
    this->_interactive_slider_buttons.push_back(mouse_event);
    return;
}

void cSDL_ManualSlider::clear_interactive_buttons_with_slider()
{
    this->_interactive_slider_buttons.clear();
    return;
}

void cSDL_ManualSlider::enable_slider_line_interaction_(bool enable)
{
    this->_enable_line_interaction = enable;
    return;
}

void cSDL_ManualSlider::enable_mouse_position_scaling_mechanism(bool enable)
{
    this->_enable_scaling_mechanism = enable;
    this->_scaling_variable_x = 1.0;
    this->_scaling_variable_y = 1.0;
    return;
}

void cSDL_ManualSlider::update_scaling_mechanism(uint32_t original_window_w,uint32_t original_window_h,uint32_t new_window_w,uint32_t new_window_h)
{
    this->_scaling_variable_x = (double)((double)new_window_w/(double)original_window_w);
    this->_scaling_variable_y = (double)((double)new_window_h/(double)original_window_h);
    return;
}

SDL_Rect cSDL_ManualSlider::get_slider_position() const
{
    if(this->_positions.size()==0)
    {
        SDL_Rect rect = {0,0,0,0};
        return rect;
    }
    else return this->_positions[this->_actual_slider_position_index];
}

uint32_t cSDL_ManualSlider::get_slider_position_index() const
{
    return this->_actual_slider_position_index;
}

 size_t cSDL_ManualSlider::get_slider_position_index_count() const
{
    return this->_positions.size();
}

bool cSDL_ManualSlider::get_slider_focus() const
{
    return this->_slider_focus;
}

bool cSDL_ManualSlider::get_slider_held_state() const
{
    return this->_slider_held;
}

bool cSDL_ManualSlider::get_slider_release_state()
{
    bool return_value = this->_button_has_been_released;
    this->_button_has_been_released = 0;
    return return_value;
}


void cSDL_ManualSlider::run_checker(SDL_Event* event)
{
    if(this->_positions.size()==0) return;  // it's needed;
    //if(this->_interactive_slider_buttons.size()==0) return;

    // check if mouse is at slider;
    int32_t mouse_x,mouse_y;
    SDL_GetMouseState(&mouse_x,&mouse_y);

    int32_t _x = this->_positions[this->_actual_slider_position_index].x;
    int32_t _y = this->_positions[this->_actual_slider_position_index].y;
    int32_t _w = this->_positions[this->_actual_slider_position_index].w;
    int32_t _h = this->_positions[this->_actual_slider_position_index].h;
    if(this->_enable_scaling_mechanism==1)
    {
        _x*=this->_scaling_variable_x; _w*=this->_scaling_variable_x;
        _y*=this->_scaling_variable_y; _h*=this->_scaling_variable_y;
    }
    if((mouse_x>=_x&&mouse_x<=(_x+_w))&&(mouse_y>=_y&&mouse_y<=(_y+_h)))
    {
        this->_slider_focus = 1;
    }
    else this->_slider_focus = 0;



    if(this->_slider_held==1)
    {
        if(event->type==SDL_MOUSEBUTTONUP)
        {
            if(event->button.button==this->_pressed_down_button)
            {
                this->_slider_held = 0;
                this->_button_has_been_released = 1;
            }
        }
        else
        {
            this->_slider_focus = 1;
            size_t size_positions = this->_positions.size();
            int32_t nearest_x = -1;
            int32_t nearest_y = -1;
            for(size_t j = 0; j<size_positions; j++)
            {
                int32_t x = this->_positions[j].x;
                int32_t y = this->_positions[j].y;
                int32_t w = this->_positions[j].w;
                int32_t h = this->_positions[j].h;
                if(this->_enable_scaling_mechanism==1)
                {
                    x*=this->_scaling_variable_x; w*=this->_scaling_variable_x;
                    y*=this->_scaling_variable_y; h*=this->_scaling_variable_y;
                }
                if((mouse_x>=x&&mouse_x<=(x+w))&&(mouse_y>=y&&mouse_y<=(y+h)))
                {
                    x+=(w/2);   // center of the slider image;
                    y+=(h/2);   // center of the slider image;
                    if(nearest_x==-1)
                    {
                        nearest_x = abs(mouse_x-x);
                        nearest_y = abs(mouse_y-y);
                        this->_actual_slider_position_index = j;
                    }
                    else
                    {
                        int32_t new_x_difference = abs(mouse_x-x);
                        int32_t new_y_difference = abs(mouse_y-y);
                        if((new_x_difference+new_y_difference)<(nearest_x+nearest_y))
                        {
                            nearest_x = new_x_difference;
                            nearest_y = new_y_difference;

                            this->_actual_slider_position_index = j;
                        }
                    }
                }
            }
        }
    }
    else if(this->_slider_focus!=0)
    {
        // check if mouse pressed at the slider;
        if(event->type==SDL_MOUSEBUTTONDOWN)
        {
            size_t size = this->_interactive_slider_buttons.size();
            for(size_t i = 0; i<size; i++)
            {
                if(event->button.button==this->_interactive_slider_buttons[i])
                {
                    this->_pressed_down_button = this->_interactive_slider_buttons[i];
                    this->_slider_held = 1;

                    size_t size_positions = this->_positions.size();
                    int32_t nearest_x = -1;
                    int32_t nearest_y = -1;
                    for(size_t j = 0; j<size_positions; j++)
                    {
                        int32_t x = this->_positions[j].x;
                        int32_t y = this->_positions[j].y;
                        int32_t w = this->_positions[j].w;
                        int32_t h = this->_positions[j].h;
                        if(this->_enable_scaling_mechanism==1)
                        {
                            x*=this->_scaling_variable_x; w*=this->_scaling_variable_x;
                            y*=this->_scaling_variable_y; h*=this->_scaling_variable_y;
                        }
                        if((mouse_x>=x&&mouse_x<=(x+w))&&(mouse_y>=y&&mouse_y<=(y+h)))
                        {
                            x+=(w/2);   // center of the slider image;
                            y+=(h/2);   // center of the slider image;
                            if(nearest_x==-1)
                            {
                                nearest_x = abs(mouse_x-x);
                                nearest_y = abs(mouse_y-y);
                                this->_actual_slider_position_index = j;
                            }
                            else
                            {
                                int32_t new_x_difference = abs(mouse_x-x);
                                int32_t new_y_difference = abs(mouse_y-y);
                                if((new_x_difference+new_y_difference)<(nearest_x+nearest_y))
                                {
                                    nearest_x = new_x_difference;
                                    nearest_y = new_y_difference;

                                    this->_actual_slider_position_index = j;
                                }
                            }
                        }
                    }

                    break;
                }
            }
        }

    }
    else
    {
        // check if mouse pressed at the point;
        if(event->type==SDL_MOUSEBUTTONDOWN)
        {
            if(this->_enable_line_interaction==1)
            {
                size_t size = this->_interactive_slider_buttons.size();
                for(size_t i = 0; i<size; i++)
                {
                    if(event->button.button==this->_interactive_slider_buttons[i])
                    {
                        this->_pressed_down_button = this->_interactive_slider_buttons[i];

                        size_t size_positions = this->_positions.size();
                        int32_t nearest_x = -1;
                        int32_t nearest_y = -1;
                        for(size_t j = 0; j<size_positions; j++)
                        {
                            int32_t x = this->_positions[j].x;
                            int32_t y = this->_positions[j].y;
                            int32_t w = this->_positions[j].w;
                            int32_t h = this->_positions[j].h;
                            if(this->_enable_scaling_mechanism==1)
                            {
                                x*=this->_scaling_variable_x; w*=this->_scaling_variable_x;
                                y*=this->_scaling_variable_y; h*=this->_scaling_variable_y;
                            }
                            if((mouse_x>=x&&mouse_x<=(x+w))&&(mouse_y>=y&&mouse_y<=(y+h)))
                            {
                                x+=(w/2);   // center of the slider image;
                                y+=(h/2);   // center of the slider image;
                                if(nearest_x==-1)
                                {
                                    nearest_x = abs(mouse_x-x);
                                    nearest_y = abs(mouse_y-y);
                                    this->_actual_slider_position_index = j;
                                    this->_slider_focus = 1;
                                    this->_slider_held  = 1;
                                }
                                else
                                {
                                    int32_t new_x_difference = abs(mouse_x-x);
                                    int32_t new_y_difference = abs(mouse_y-y);
                                    if((new_x_difference+new_y_difference)<(nearest_x+nearest_y))
                                    {
                                        nearest_x = new_x_difference;
                                        nearest_y = new_y_difference;

                                        this->_actual_slider_position_index = j;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    return;
}

//      cSDL class for automatic horizontal or vertical slider; class isn't fully idiot-resistant;
// needs class "CustomVector";
// #include <SDL2/SDL.h>
// #include <stdint.h>
// #include <stdlib.h>
class cSDL_AutomaticSlider
{
public:
    enum MouseValues {MOUSE_BUTTON_RIGHT = SDL_BUTTON_RIGHT,MOUSE_BUTTON_LEFT = SDL_BUTTON_LEFT,
                            MOUSE_BUTTON_MIDDLE = SDL_BUTTON_MIDDLE,MOUSE_BUTTON_X1 = SDL_BUTTON_X1,MOUSE_BUTTON_X2 = SDL_BUTTON_X2};
    enum TypeOfSlider {HORIZONTAL,VERTICAL};

//      constructor;
//  "type_of_slider": decides what kind of slider u want to build and basically decides how passed argument to functions and class as a whole will be reacting;
//  "starting_x": beginning position of x of the slider;
//  "starting_y": beginning position of y of the slider;
//  "width": width of the slider; it's an especially important when type_of_slider==HORIZONTAL 'cause then variable is used for calculating free space between starting hitbox of the slider and
//      maximum end position/larger segment of the slider; if variable is 0 then it will be automatically changed to 1;
//  "height: height of the slider; if variable is 0 then it will be automatically changed to 1;
//  "right_x_or_upper_y": depending on "type_of_slider" that should be most right or most upper possible position of the slider; if this variable is higher than (starting_y-1)
//      when type_of_slider==VERTICAL,then variable will be changed automatically to (starting_y-1); or if this variable is lower than (starting_x+width+1) when type_of_slider==HORIZONTAL then
//      variable will be changed automatically to (starting_x+width+1);
//  "slider_segments_count": number of possible positions available for the slider to move to; divides the available free space into your number of segments which are used by the slider to move between them;
//      if type_of_slider==HORIZONTAL and variable is higher than (right_x_or_upper_y-(starting_x+width)) then it will automatically become this maximum variable;
//      else if type_of_slider==VERTICAL and variable is higher than (starting_y-right_x_or_upper_y) then it will automatically become this variable;
//      if variable is lower than 1 then it will automatically become 1;
    cSDL_AutomaticSlider(cSDL_AutomaticSlider::TypeOfSlider type_of_slider,int32_t starting_x,int32_t starting_y,uint32_t width,uint32_t height,int32_t right_x_or_upper_y,uint32_t slider_segments_count);

//      function to change slider parameters that u gave to the constructor at first; every argument is the same like in the constructor so u can read constructor comments about these variables;
    void change_slider_parameters(cSDL_AutomaticSlider::TypeOfSlider type_of_slider,int32_t starting_x,int32_t starting_y,uint32_t width,uint32_t height,int32_t right_x_or_upper_y,uint32_t slider_segments_count);

//      to enable scaling mechanism for segments to interact with mouse coordinates;
//          by enabling this mechanism by this function,u can use function "update_scaling_mechanism()";
//          by default it's 0 (OFF); scaling variable resets after running this function so u have to run "update_scaling_mechanism()" after calling this func;
//  "enable": 1 to enable,0 to disable;
    void enable_mouse_position_scaling_mechanism(bool enable);
//      to update scaling mechanism;
//  "original_window_w"&"original_window_h": ur original window dimensions;
//  "new_window_w"&"new_window_h": ur new window dimensions;
    void update_scaling_mechanism(uint32_t original_window_w,uint32_t original_window_h,uint32_t new_window_w,uint32_t new_window_h);

//      to enable/disable interaction with hitboxes of the line; by default it's 1 (ON);
    void enable_slider_line_interaction_(bool enable);

//      set manually specify segment position for the slider; if semgnet is too large then segment will be set to the maximum possible;
    void set_slider_actual_segment(uint32_t segment);
//      increase actual slider position segment; if number is too large then number will be set to the maximum possible;
    void increase_actual_slider_segment(uint32_t increase_by);
//      the same function as above except function decreases actual segment; if number is too large then number will be set to the maximum possible;
    void decrease_actual_slider_segment(uint32_t decrease_by);

//      adds event to actual events that can interact with slider; by default there is 1 button that can interact with slider and that is "MOUSE_BUTTON_LEFT";
//  "mouse_event": button that u want to add to a pool of interactive buttons with a slider;
    void add_interactive_button(cSDL_AutomaticSlider::MouseValues mouse_event);
//      clear all of the buttons of the pool that can interact with the slider;
    void clear_interactive_buttons();

//      get actual X or Y of slider; should be called after "run_checker()";
//  returns X or Y of slider; it depends on what type of slider "TypeOfSlider" u have build; if it's "HORIZONTAL" then func will return X,otherwise it will return Y;
    int32_t get_slider_x_y() const;
//      get actual slider segment index; should be called after "run_cheker()";
//  returns segment position;
    uint32_t get_slider_actual_segment() const;
//      get actual count of segments of the slider; should be called after "run_cheker()";
//  returns number of how much segments slider has minus starting segment;
    size_t get_slider_count_of_segments() const;
//      get actual state of slider focus; should be called after "run_checker()";
//  returns state of slider focus; returns 0 if there is no focus,returns 1 if there is focus;
    bool get_slider_focus() const;
//      get actual state of slider held; should be called after "run_checker()";
//  returns 0 if slider isn't held; returns 1 if slider is held;
    bool get_slider_held_state() const;
//      get information if mouse button has been release; everytime this function is called,everytime variable about release state is reseted;
//  returns 1 if release state has been confirmed; returns 0 if there was not any new release states;
    bool get_slider_release_state();


//      main function; should be called everytime u calls "SDL_PollEvent()";
//  "event": updated event variable by function "SDL_PollEvent()";
    void run_checker(SDL_Event* event);

private:
    struct MousePoint {int32_t x; int32_t y;};
    CustomVector<MouseValues> _interactive_buttons;  // by default there's only "MOUSE_BUTTON_LEFT"; remember to check if this variable size() isn't 0;
    CustomVector<int32_t> _slider_segments;
    int32_t _slider_start_x;
    int32_t _slider_start_y;
    uint32_t _slider_width;
    uint32_t _slider_height;
    int32_t _slider_end_x_y;
    uint32_t _slider_segments_count;    //  size of "_slider_segments" but minus slider starting position;
    TypeOfSlider _slider_type;
    double _scaling_variable_x;         // it's only matters when "_enable_scaling_mechanism" is 1; it's a calculation variable;
    double _scaling_variable_y;         // it's only matters when "_enable_scaling_mechanism" is 1; it's a calculation variable;
    MouseValues _pressed_down_button;   // it's a calculation variable;
    bool _enable_line_interaction;      // by default it's 1 (ON);
    MousePoint _mouse_hold_point;        // it's a calculation variable;
    uint32_t _segment_held_index;       // it's a calculation variable;

    uint32_t _actual_slider_segment;    // actual slider segment from "_slider_segments";
    bool _slider_focus; // does slider has mouse focsu;
    bool _slider_held;  // does slider keep track of the mouse/is moving by the mouse;
    bool _enable_scaling_mechanism;     // by default it's 0 (OFF);
    bool _button_has_been_released;     // info about release state of mouse button;

    void _recalculate_slider_segments();

};

cSDL_AutomaticSlider::cSDL_AutomaticSlider(cSDL_AutomaticSlider::TypeOfSlider type_of_slider,int32_t starting_x,int32_t starting_y,uint32_t width,uint32_t height,int32_t right_x_or_upper_y,uint32_t slider_segments_count)
{
    this->_slider_start_x = starting_x;
    this->_slider_start_y = starting_y;
    if(width==0) this->_slider_width = 1;
    else this->_slider_width = width;
    if(height==0) this->_slider_height = 1;
    else this->_slider_height = height;
    this->_slider_type = type_of_slider;
    if(slider_segments_count==0) slider_segments_count = 1;

    if(type_of_slider==TypeOfSlider::HORIZONTAL)
    {
        if((this->_slider_start_x+(int32_t)this->_slider_width+1)>right_x_or_upper_y) this->_slider_end_x_y = (this->_slider_start_x+this->_slider_width+1);
        else this->_slider_end_x_y = right_x_or_upper_y;

        if(abs(this->_slider_end_x_y-(this->_slider_start_x+(int32_t)this->_slider_width))<(int32_t)slider_segments_count) this->_slider_segments_count = abs(this->_slider_end_x_y-(this->_slider_start_x+(int32_t)this->_slider_width));
        else this->_slider_segments_count = slider_segments_count;
    }
    else
    {
        if((this->_slider_start_y-1)<right_x_or_upper_y) this->_slider_end_x_y = (this->_slider_start_y-1);
        else this->_slider_end_x_y = right_x_or_upper_y;

        if(abs(this->_slider_start_y-this->_slider_end_x_y)<(int32_t)slider_segments_count) this->_slider_segments_count = abs(this->_slider_start_y-this->_slider_end_x_y);
        else this->_slider_segments_count = slider_segments_count;
    }

    this->_recalculate_slider_segments();

    this->_interactive_buttons.push_back(cSDL_AutomaticSlider::MouseValues::MOUSE_BUTTON_LEFT);
    this->_actual_slider_segment = 0;
    this->_slider_focus = 0;
    this->_slider_held = 0;
    this->_enable_scaling_mechanism = 0;
    this->_button_has_been_released = 0;
    this->_enable_line_interaction = 1;

    //printf("%d %d %d %d\n",this->_slider_start_x_y,this->_slider_width,this->_slider_end_x_y,this->_slider_segments_count);


    return;
}

void cSDL_AutomaticSlider::change_slider_parameters(cSDL_AutomaticSlider::TypeOfSlider type_of_slider,int32_t starting_x,int32_t starting_y,uint32_t width,uint32_t height,int32_t right_x_or_upper_y,uint32_t slider_segments_count)
{
    this->_slider_start_x = starting_x;
    this->_slider_start_y = starting_y;
    if(width==0) this->_slider_width = 1;
    else this->_slider_width = width;
    if(height==0) this->_slider_height = 1;
    else this->_slider_height = height;
    this->_slider_type = type_of_slider;
    if(slider_segments_count==0) slider_segments_count = 1;

    if(type_of_slider==TypeOfSlider::HORIZONTAL)
    {
        if((this->_slider_start_x+(int32_t)this->_slider_width+1)>right_x_or_upper_y) this->_slider_end_x_y = (this->_slider_start_x+this->_slider_width+1);
        else this->_slider_end_x_y = right_x_or_upper_y;

        if(abs(this->_slider_end_x_y-(this->_slider_start_x+(int32_t)this->_slider_width))<(int32_t)slider_segments_count) this->_slider_segments_count = abs(this->_slider_end_x_y-(this->_slider_start_x+(int32_t)this->_slider_width));
        else this->_slider_segments_count = slider_segments_count;
    }
    else
    {
        if((this->_slider_start_y-1)<right_x_or_upper_y) this->_slider_end_x_y = (this->_slider_start_y-1);
        else this->_slider_end_x_y = right_x_or_upper_y;

        if(abs(this->_slider_start_y-this->_slider_end_x_y)<(int32_t)slider_segments_count) this->_slider_segments_count = abs(this->_slider_start_y-this->_slider_end_x_y);
        else this->_slider_segments_count = slider_segments_count;
    }

    this->_recalculate_slider_segments();

    if(this->_actual_slider_segment>this->_slider_segments_count) this->_actual_slider_segment = this->_slider_segments_count;
    this->_slider_held = 0; // if slider is held right now;

    return;
}

void cSDL_AutomaticSlider::enable_mouse_position_scaling_mechanism(bool enable)
{
    this->_enable_scaling_mechanism = enable;
    this->_scaling_variable_x = 1.0;
    this->_scaling_variable_y = 1.0;
    return;
}

void cSDL_AutomaticSlider::update_scaling_mechanism(uint32_t original_window_w,uint32_t original_window_h,uint32_t new_window_w,uint32_t new_window_h)
{
    this->_scaling_variable_x = (double)((double)new_window_w/(double)original_window_w);
    this->_scaling_variable_y = (double)((double)new_window_h/(double)original_window_h);
    return;
}

void cSDL_AutomaticSlider::enable_slider_line_interaction_(bool enable)
{
    this->_enable_line_interaction = enable;
    return;
}



void cSDL_AutomaticSlider::set_slider_actual_segment(uint32_t segment)
{
    if(segment>this->_slider_segments_count) this->_actual_slider_segment = this->_slider_segments_count;
    else this->_actual_slider_segment = segment;
    return;
}

void cSDL_AutomaticSlider::increase_actual_slider_segment(uint32_t increase_by)
{
    if((this->_actual_slider_segment+increase_by)>this->_slider_segments_count) this->_actual_slider_segment = this->_slider_segments_count;
    else this->_actual_slider_segment+=increase_by;
    return;
}

void cSDL_AutomaticSlider::decrease_actual_slider_segment(uint32_t decrease_by)
{
    if(decrease_by>this->_actual_slider_segment) this->_actual_slider_segment = 0;
    else this->_actual_slider_segment-=decrease_by;
    return;
}

void cSDL_AutomaticSlider::add_interactive_button(cSDL_AutomaticSlider::MouseValues mouse_event)
{
    this->_interactive_buttons.push_back(mouse_event);
    return;
}

void cSDL_AutomaticSlider::clear_interactive_buttons()
{
    this->_interactive_buttons.clear();
    return;
}

int32_t cSDL_AutomaticSlider::get_slider_x_y() const
{
    if(this->_slider_type==TypeOfSlider::HORIZONTAL)
    {
        return this->_slider_segments[this->_actual_slider_segment]-this->_slider_width;
    }
    else
    {
        return this->_slider_segments[this->_actual_slider_segment];
    }
}

uint32_t cSDL_AutomaticSlider::get_slider_actual_segment() const
{
    return this->_actual_slider_segment;
}

uint32_t cSDL_AutomaticSlider::get_slider_count_of_segments() const
{
    return this->_slider_segments_count;
}

bool cSDL_AutomaticSlider::get_slider_focus() const
{
    return this->_slider_focus;
}

bool cSDL_AutomaticSlider::get_slider_held_state() const
{
    return this->_slider_held;
}

bool cSDL_AutomaticSlider::get_slider_release_state()
{
    bool variable = this->_button_has_been_released;
    this->_button_has_been_released = 0;
    return variable;
}

void cSDL_AutomaticSlider::run_checker(SDL_Event* event)
{
    // check if mouse is at slider;
    int32_t mouse_x,mouse_y;
    SDL_GetMouseState(&mouse_x,&mouse_y);

    if(this->_slider_type==TypeOfSlider::HORIZONTAL)
    {
        int32_t _x = this->_slider_segments[this->_actual_slider_segment]-this->_slider_width;
        int32_t _y = this->_slider_start_y;
        int32_t _w = (int32_t)this->_slider_width;
        int32_t _h = (int32_t)this->_slider_height;
        if(this->_enable_scaling_mechanism==1)
        {
            _x*=this->_scaling_variable_x; _w*=this->_scaling_variable_x;
            _y*=this->_scaling_variable_y; _h*=this->_scaling_variable_y;
        }
        if((mouse_x>=_x&&mouse_x<=(_x+_w))&&(mouse_y>=_y&&mouse_y<=(_y+_h)))
        {
            this->_slider_focus = 1;
        }
        else this->_slider_focus = 0;


        if(this->_slider_held==1)
        {
            if(event->type==SDL_MOUSEBUTTONUP)
            {
                if(event->button.button==this->_pressed_down_button)
                {
                    this->_slider_held = 0;
                    this->_button_has_been_released = 1;
                }
            }
            else
            {
                this->_slider_focus = 1;
                size_t size_segments = this->_slider_segments.size();
                int32_t x_difference = mouse_x-this->_mouse_hold_point.x;
                if(x_difference<0)  // left;
                {
                    for(int32_t j = (this->_segment_held_index); j>=0; j--)
                    {
                        int32_t x_shift = this->_slider_segments[j];
                        int32_t x_base = this->_slider_segments[this->_segment_held_index];
                        if(this->_enable_scaling_mechanism==1)
                        {
                            x_base*=this->_scaling_variable_x; x_shift*=this->_scaling_variable_x;
                        }
                        if((x_difference)<=(x_shift-x_base))
                        {
                            this->_actual_slider_segment = j;
                        }
                        else break;
                    }
                }
                else if(x_difference>0) // right;
                {
                    for(size_t j = (this->_segment_held_index); j<size_segments; j++)
                    {
                        int32_t x_shift = this->_slider_segments[j];
                        int32_t x_base = this->_slider_segments[this->_segment_held_index];
                        if(this->_enable_scaling_mechanism==1)
                        {
                            x_base*=this->_scaling_variable_x; x_shift*=this->_scaling_variable_x;
                        }
                        if(x_difference>=(x_shift-x_base))
                        {
                            this->_actual_slider_segment = j;
                        }
                        else break;
                    }
                }
                else
                {
                    this->_actual_slider_segment = this->_segment_held_index;
                }



            }
        }
        else if(this->_slider_focus==1)
        {
            // check if mouse pressed at the slider;
            if(event->type==SDL_MOUSEBUTTONDOWN)
            {
                size_t size = this->_interactive_buttons.size();
                for(size_t i = 0; i<size; i++)
                {
                    if(event->button.button==this->_interactive_buttons[i])
                    {
                        this->_pressed_down_button = this->_interactive_buttons[i];
                        this->_slider_held = 1;
                        this->_mouse_hold_point.x = mouse_x;
                        this->_segment_held_index = this->_actual_slider_segment;

                        break;
                    }
                }
            }
        }
        else
        {
            // check if mouse pressed at the point;
            if(event->type==SDL_MOUSEBUTTONDOWN)
            {
                if(this->_enable_line_interaction==1)
                {
                    size_t size = this->_interactive_buttons.size();
                    for(size_t i = 0; i<size; i++)
                    {
                        if(event->button.button==this->_interactive_buttons[i])
                        {
                            this->_pressed_down_button = this->_interactive_buttons[i];
                            this->_mouse_hold_point.x = mouse_x;

                            size_t size_segments = this->_slider_segments.size();
                            int32_t nearest_x = -1;
                            for(size_t j = 0; j<size_segments; j++)
                            {
                                int32_t x = this->_slider_segments[j]-this->_slider_width;
                                int32_t y = this->_slider_start_y;
                                int32_t w = (int32_t)this->_slider_width;
                                int32_t h = (int32_t)this->_slider_height;
                                if(this->_enable_scaling_mechanism==1)
                                {
                                    x*=this->_scaling_variable_x; w*=this->_scaling_variable_x;
                                    y*=this->_scaling_variable_y; h*=this->_scaling_variable_y;
                                }
                                if((mouse_x>=x&&mouse_x<=(x+w))&&(mouse_y>=y&&mouse_y<=(y+h)))
                                {
                                    x+=(w/2);   // center of the slider image;
                                    if(nearest_x==-1)
                                    {
                                        nearest_x = abs(mouse_x-x);
                                        this->_actual_slider_segment = j;
                                        this->_slider_focus = 1;
                                        this->_slider_held  = 1;
                                        this->_segment_held_index = j;
                                    }
                                    else
                                    {
                                        int32_t new_x_difference = abs(mouse_x-x);
                                        if((new_x_difference)<(nearest_x))
                                        {
                                            nearest_x = new_x_difference;

                                            this->_actual_slider_segment = j;
                                            this->_segment_held_index = j;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        int32_t _x = this->_slider_start_x;
        int32_t _y = this->_slider_segments[this->_actual_slider_segment];
        int32_t _w = (int32_t)this->_slider_width;
        int32_t _h = (int32_t)this->_slider_height;
        if(this->_enable_scaling_mechanism==1)
        {
            _x*=this->_scaling_variable_x; _w*=this->_scaling_variable_x;
            _y*=this->_scaling_variable_y; _h*=this->_scaling_variable_y;
        }
        if((mouse_x>=_x&&mouse_x<=(_x+_w))&&(mouse_y>=_y&&mouse_y<=(_y+_h)))
        {
            this->_slider_focus = 1;
        }
        else this->_slider_focus = 0;


        if(this->_slider_held==1)
        {
            if(event->type==SDL_MOUSEBUTTONUP)
            {
                if(event->button.button==this->_pressed_down_button)
                {
                    this->_slider_held = 0;
                    this->_button_has_been_released = 1;
                }
            }
            else
            {
                this->_slider_focus = 1;
                size_t size_segments = this->_slider_segments.size();
                int32_t y_difference = mouse_y-this->_mouse_hold_point.y;
                if(y_difference>0)  // down
                {
                    for(int32_t j = (this->_segment_held_index); j>=0; j--)
                    {
                        int32_t y_shift = this->_slider_segments[j];
                        int32_t y_base = this->_slider_segments[this->_segment_held_index];
                        if(this->_enable_scaling_mechanism==1)
                        {
                            y_base*=this->_scaling_variable_y; y_shift*=this->_scaling_variable_y;
                        }
                        if((y_difference)>=(y_shift-y_base))
                        {
                            this->_actual_slider_segment = j;
                        }
                        else break;
                    }
                }
                else if(y_difference<0) // up
                {
                    for(size_t j = (this->_segment_held_index); j<size_segments; j++)
                    {
                        int32_t y_shift = this->_slider_segments[j];
                        int32_t y_base = this->_slider_segments[this->_segment_held_index];
                        if(this->_enable_scaling_mechanism==1)
                        {
                            y_base*=this->_scaling_variable_y; y_shift*=this->_scaling_variable_y;
                        }
                        if((y_difference)<=(y_shift-y_base))
                        {
                            this->_actual_slider_segment = j;
                        }
                        else break;
                    }
                }
                else
                {
                    this->_actual_slider_segment = this->_segment_held_index;
                }



            }
        }
        else if(this->_slider_focus==1)
        {
            // check if mouse pressed at the slider;
            if(event->type==SDL_MOUSEBUTTONDOWN)
            {
                size_t size = this->_interactive_buttons.size();
                for(size_t i = 0; i<size; i++)
                {
                    if(event->button.button==this->_interactive_buttons[i])
                    {
                        this->_pressed_down_button = this->_interactive_buttons[i];
                        this->_slider_held = 1;
                        this->_mouse_hold_point.y = mouse_y;
                        this->_segment_held_index = this->_actual_slider_segment;

                        break;
                    }
                }
            }
        }
        else
        {
            // check if mouse pressed at the point;
            if(event->type==SDL_MOUSEBUTTONDOWN)
            {
                if(this->_enable_line_interaction==1)
                {
                    size_t size = this->_interactive_buttons.size();
                    for(size_t i = 0; i<size; i++)
                    {
                        if(event->button.button==this->_interactive_buttons[i])
                        {
                            this->_pressed_down_button = this->_interactive_buttons[i];
                            this->_mouse_hold_point.y = mouse_y;

                            size_t size_segments = this->_slider_segments.size();
                            int32_t nearest_y = -1;
                            for(size_t j = 0; j<size_segments; j++)
                            {
                                int32_t x = this->_slider_start_x;
                                int32_t y = this->_slider_segments[j];
                                int32_t w = (int32_t)this->_slider_width;
                                int32_t h = (int32_t)this->_slider_height;
                                if(this->_enable_scaling_mechanism==1)
                                {
                                    x*=this->_scaling_variable_x; w*=this->_scaling_variable_x;
                                    y*=this->_scaling_variable_y; h*=this->_scaling_variable_y;
                                }
                                if((mouse_x>=x&&mouse_x<=(x+w))&&(mouse_y>=y&&mouse_y<=(y+h)))
                                {
                                    y+=(h/2);   // center of the slider image;
                                    if(nearest_y==-1)
                                    {
                                        nearest_y = abs(mouse_y-y);
                                        this->_actual_slider_segment = j;
                                        this->_slider_focus = 1;
                                        this->_slider_held  = 1;
                                        this->_segment_held_index = j;
                                    }
                                    else
                                    {
                                        int32_t new_y_difference = abs(mouse_y-y);
                                        if((new_y_difference)<(nearest_y))
                                        {
                                            nearest_y = new_y_difference;

                                            this->_actual_slider_segment = j;
                                            this->_segment_held_index = j;
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    return;
}

void cSDL_AutomaticSlider::_recalculate_slider_segments()
{
    this->_slider_segments.clear();
    this->_slider_segments.reserve(this->_slider_segments_count+1);

    if(this->_slider_type==TypeOfSlider::HORIZONTAL)
    {
        const int32_t slider_right = this->_slider_start_x+this->_slider_width;
        this->_slider_segments.push_back(slider_right);
        double segment_shift_value = (((double)abs(this->_slider_end_x_y-slider_right))/(double)this->_slider_segments_count);
        double actual_segment_value = slider_right+segment_shift_value;
        for(size_t i = 1; i<this->_slider_segments_count; i++)
        {
            this->_slider_segments.push_back((int32_t)actual_segment_value);
            actual_segment_value+=segment_shift_value;
        }
        this->_slider_segments.push_back(this->_slider_end_x_y);
    }
    else
    {
        this->_slider_segments.push_back(this->_slider_start_y);
        double segment_shift_value = (((double)abs(this->_slider_start_y-this->_slider_end_x_y))/(double)this->_slider_segments_count);
        double actual_segment_value = this->_slider_start_y-segment_shift_value;
        for(size_t i = 1; i<this->_slider_segments_count; i++)
        {
            this->_slider_segments.push_back((int32_t)actual_segment_value);
            actual_segment_value-=segment_shift_value;
        }
        this->_slider_segments.push_back(this->_slider_end_x_y);
    }

    /*for(size_t i = 0; i<this->_slider_segments.size(); i++)
    {
        printf("%d\n",this->_slider_segments[i]);
    }*/

    return;
}
