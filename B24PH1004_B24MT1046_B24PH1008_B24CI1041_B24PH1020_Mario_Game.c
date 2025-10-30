#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define width 900
#define height 800
#define ground 600
#define boy_speed 7
#define jump 20
#define gravity 1
#define coin_speed 5
#define coin_number 3
#define flying_speed 5
#define enemy_speed -8
#define value_coin 10
#define score_file "B24PH1004_B24MT1046_B24PH1008_B24CI1041_B24PH1021_Score.txt"
#define number_of_score 50
#define red_coin_number 1

#define number_of_bullets 10  
#define bullet_speed 5   

SDL_Texture* bg_texture = NULL;
 

 
 
typedef struct basic {
    int x, y, w, h;
    int speedX, speedY;
    int jumping;
    int radius;
} basic;


typedef struct {
    int x, y, w, h;
    int speedX, speedY;
    int active; 
} Projectile;

Projectile bullets[number_of_bullets];

// global varioable 
 SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
 int gameover = 0;
int score = 0;
int life = 3;
int topScores[number_of_score];
basic boy, enemy, flying_enemy;
basic coins[coin_number];
basic red[red_coin_number];
 

// Function prototypes
int error();
void reset_game();
void input(SDL_Event e);
void update_game();
void render_game();
void  read_top_scores();
void update_top_scores(int newScore);
void print_top_scores();
void closeSDL();
 void game_run();
 

// Initialize SDL error found ya not this function that thing if any error found function give error 
int error() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Initialization failed! Error is : %s\n", SDL_GetError());
        return 0;
    }

    window = SDL_CreateWindow("Mario :)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window creation failed! Error: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL) {
        printf("Renderer creation failed!   Error: %s\n", SDL_GetError());
        return 0;
    }

    // load background image heree 
    SDL_Surface* background = SDL_LoadBMP("B24PH1004_B24MT1046_B24PH1008_B24CI1041_B24PH1020_Backgroun_Image.bmp");

    if (background==NULL) {
        printf("Failed to load background image: %s\n", SDL_GetError());
        return 0;
    }

  bg_texture = SDL_CreateTextureFromSurface(renderer, background);
    SDL_FreeSurface(background);

    if (!bg_texture) {
        printf("Failed to create background texture: %s\n", SDL_GetError());
        return 0;
    }

    return 1;
}

 // game to reset karne ke liye
void reset_game() {

    gameover = 0; 
    // boy coordinate fix 
    boy.x = 100;
    boy.y = ground;
    boy.w = 30;
    boy.h = 50;
    boy.speedX = 0;
    boy.speedY = 0;
    boy.jumping = 0;

    // enemy coordinate fix
    enemy.x = width - 100;
    enemy.y = ground;
    enemy.w = 50;
    enemy.h = 50;
    enemy.speedX = enemy_speed;

    //  coin coordinate change with time and random change
    srand(time(NULL));
    for (int i = 0; i < coin_number; i++) {
        coins[i].x = rand() % width;
        coins[i].y = rand() % 100;
        coins[i].radius = 10;
        coins[i].speedY = coin_speed;
    }

    // red coin coordinat fix
    srand(time(NULL));
    for (int i = 0; i < red_coin_number; i++) {
        red[i].x = rand() % width;
        red[i].y = rand() % 100;
        red[i].radius = 15;
        red[i].speedY = coin_speed;
    }

    // flying_enemy coordinate
    flying_enemy.x = 200;
    flying_enemy.y = ground - 140;
    flying_enemy.w = 40;
    flying_enemy.h = 20;
    flying_enemy.speedX = flying_speed;
    flying_enemy.speedY = 1;

    // all bullets inactive first
    for (int i = 0; i < number_of_bullets; i++) {
        bullets[i].active = 0;
    }


}


// player se input lekar jo speed hai us mai value dena 
void input(SDL_Event e) {

    if (e.type == SDL_QUIT) {
        gameover = 1;
    }

    else if (e.type == SDL_KEYDOWN) {

        switch (e.key.keysym.sym) {
        case SDLK_a:
            boy.speedX = -boy_speed;
            break;
        case SDLK_d:
            boy.speedX =boy_speed;
            break;
        case SDLK_w:
            if (boy.jumping != 1) {
                boy.jumping = 1;
                boy.speedY = -jump;
            }
            break;
        }
    }

    else if (e.type == SDL_KEYUP) {
        switch (e.key.keysym.sym) {
        case SDLK_a:
            boy.speedX = 0;
            break;

        case SDLK_d:
            boy.speedX = 0;
            break;
        }
    }

}

 

// user se input lene ke baad game mai player ke coordinate update karna hai
void update_game() {

    boy.x += boy.speedX;    // update x coordinate of boy
    if (boy.x < 0) boy.x = 0; // condition that boy window  se bahar nhi jana chayie
    if (boy.x + boy.w > width) boy.x = width - boy.w;

    // update y coordinate with condition
    if (boy.jumping == 1) {
        boy.y += boy.speedY;
        boy.speedY += gravity;

        if (boy.y >= ground) { // if boy out of ground then this condition ground ke niche nhi jane deti hai
            boy.y = ground;
            boy.jumping = 0;
        }
    }

    // update enemy coordinate of x
    enemy.x += enemy.speedX;
    if (enemy.x + enemy.w < 0) {  // condition that if enemy go out of window then wapas reset this
        enemy.x = width;
    }
     
    // update flying enemy coordinate of x and y
    flying_enemy.x += flying_enemy.speedX;
    flying_enemy.y += flying_enemy.speedY;

     // condition that flying enemy ek frame mai kitni baar up down hoga ye random karne ke liye
    if (rand() % 20 == 0) {  
        flying_enemy.speedY = (rand() % 3) - 1;  // random up ya down of enemy
    }

    //  condition yadi wo screen ke bahar chala gaya hai toh
    if (flying_enemy.x > width || flying_enemy.x + flying_enemy.w < 0) {

        //  random decide which side next come enemy 
        if (rand() % 2 == 0) {
            // left side se aane ke liye 
            flying_enemy.x = -flying_enemy.w;

            flying_enemy.speedX = -(flying_speed) + (rand() % 5); //  random speed change of enemy 
        }
        else {
            //   right side se aane ke liye
            flying_enemy.x = width;
            flying_enemy.speedX = -(flying_speed - (rand() % 5)); //  random speed change of enemy
        }
        //  randomly come for any directiona any high
        flying_enemy.y = 50+ rand() % (ground - flying_enemy.h - 100);
        flying_enemy.speedY = (rand() % 3) - 1;

    }


    // Shooting condition: random chance per frame to shoot a bullet
    if (rand() % 100 == 0) { // roughly 1 in 100 chance per frame; adjust as needed
        // Find an available bullet slot
        for (int i = 0; i < number_of_bullets; i++) {
            if (bullets[i].active==0) {
                
                bullets[i].active = 1;

                // set coordinate of bullets
                bullets[i].x = flying_enemy.x + flying_enemy.w / 2;
                bullets[i].y = flying_enemy.y + flying_enemy.h / 2;
                bullets[i].w = 10;
                bullets[i].h = 10;
                 
                // check where boy of with respect of flying enemy
                if (flying_enemy.x < boy.x) {
                    bullets[i].speedX = bullet_speed;  
                }
                else {
                    bullets[i].speedX = -bullet_speed;  
                }
                //  random speed change of bullent
                bullets[i].speedY = (rand() % 3) - 1;   
                break;  
            }
        }
    }

    SDL_Rect boy_rect = { boy.x, boy.y, boy.w, boy.h };


    for (int i = 0; i < number_of_bullets; i++) {
        // goli chal gayi toh usko coordinate bhi toh update karni hai
        if (bullets[i].active==1) {
            // update bullet coordinate of bullet
            bullets[i].x += bullets[i].speedX;
            bullets[i].y += bullets[i].speedY;

            // deactivate the goli yadi screen ke bahar chali jaye
            if (bullets[i].x < 0 || bullets[i].x > width || bullets[i].y < 0 || bullets[i].y > height) {
                bullets[i].active = 0;
            }

            // check if bullent touch boy game over
            SDL_Rect bullet_rect = { bullets[i].x, bullets[i].y, bullets[i].w, bullets[i].h };
           
            if (SDL_HasIntersection(&bullet_rect, &boy_rect)) {
                gameover = 1;  
                bullets[i].active = 0;
            }
        }
    }



    // check if boy touch with any enemy
    
    SDL_Rect enemy_rect = { enemy.x, enemy.y, enemy.w, enemy.h };
    if (SDL_HasIntersection(&boy_rect, &enemy_rect)) {
        gameover = 1;
    }
    SDL_Rect flying_rect = { flying_enemy.x, flying_enemy.y, flying_enemy.w, flying_enemy.h };
    if (SDL_HasIntersection(&boy_rect, &flying_rect)) {
        gameover = 1;
    }

    //  coin update
    for (int i = 0; i < coin_number; i++) {
        coins[i].y += coins[i].speedY;
        if (coins[i].y - coins[i].radius > height) {
            coins[i].x = rand() % width;
            coins[i].y = -coins[i].radius;
        }
        //  condition if boy touch coin update coin and score
        int dx = (boy.x + boy.w / 2) - coins[i].x;
        int dy = (boy.y + boy.h / 2) - coins[i].y;
        int distance = dx * dx + dy * dy;
        int combined_distance = (coins[i].radius + boy.w / 2) * (coins[i].radius + boy.w / 2);
        if (distance <= combined_distance) {
            score += value_coin;
            coins[i].x = rand() % width;
            coins[i].y = -coins[i].radius;
        }
    }

    // red coin update corrdinate
    for (int i = 0; i < red_coin_number; i++) {
        red[i].y += red[i].speedY;
        if (red[i].y - red[i].radius > height) {
            red[i].x = rand() % width;
            red[i].y = -red[i].radius;
        }
        // if boy touch coin game over
        int dx = (boy.x + boy.w / 2) - red[i].x;
        int dy = (boy.y + boy.h / 2) - red[i].y;
        int distance = dx * dx + dy * dy;
        int combined_distance = (red[i].radius + boy.w / 2) * (red[i].radius + boy.w / 2);
        if (distance <= combined_distance) {
            score += value_coin*2;
           red[i].x = rand() % width;
            red[i].y = -red[i].radius;
        }
    }

}


void draw_coin(SDL_Renderer* renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}
  
 
void render_game() {
 
    // set background of image that i load first
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, bg_texture, NULL, NULL);   

    // set ground 
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    SDL_Rect ground_colour = { 0, ground + boy.h, width, height - ground };
    SDL_RenderFillRect(renderer, &ground_colour);
 
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect boy_rect = { boy.x, boy.y, boy.w, boy.h };
    SDL_RenderFillRect(renderer, &boy_rect);


    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
    SDL_Rect enemy_rect = { enemy.x, enemy.y, enemy.w, enemy.h };
    SDL_RenderFillRect(renderer, &enemy_rect);

    SDL_SetRenderDrawColor(renderer, 255, 223, 0, 255);
    for (int i = 0; i < coin_number; i++) {
        draw_coin(renderer, coins[i].x, coins[i].y, coins[i].radius);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < red_coin_number; i++) {
        draw_coin(renderer, red[i].x, red[i].y, red[i].radius);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect flyingRect = { flying_enemy.x, flying_enemy.y, flying_enemy.w, flying_enemy.h };
    SDL_RenderFillRect(renderer, &flyingRect);

    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); 
    for (int i = 0; i < number_of_bullets; i++) {
        if (bullets[i].active) {
            SDL_Rect bulletRect = { bullets[i].x, bullets[i].y, bullets[i].w, bullets[i].h };
            SDL_RenderFillRect(renderer, &bulletRect);
        }
    }


    SDL_RenderPresent(renderer);
}
 

void  read_top_scores() {
    FILE* fp = fopen(score_file, "r");
    if (fp == NULL) {
        printf("File don't open!");
    }
    else  {
        for (int i = 0; i < number_of_score; i++) {
            fscanf_s(fp, "%d", &topScores[i]);
        }
        fclose(fp);
    }
    
}


void update_top_scores(int newScore) {
     
    for (int i = 0; i < number_of_score; i++) {
        if (newScore > topScores[i]) {
             
            for (int j = number_of_score - 1; j > i; j--) {
                topScores[j] = topScores[j - 1];
            }
            
            topScores[i] = newScore;
            printf("\nCongratulations! You beat the high score.\n");
            break;
        }
    }

    // Write updated scores back to file
    FILE* fp = fopen(score_file, "w");
    if (fp != NULL) {
        for (int i = 0; i < number_of_score; i++) {
            fprintf(fp, "%d\n", topScores[i]);
        }
        fclose(fp);
    }
}


void print_top_scores() {
    printf("\n===== TOP %d SCORES =====\n",number_of_score);
    for (int i = 0; i < number_of_score; i++) {
        printf("%d. %d\n", i + 1, topScores[i]);
    }
}


// Global variables for audio
Uint8* audio_buffer;
Uint32 audio_length;
SDL_AudioDeviceID device_id;
 
int audio_error() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("Audio SDL_Init error: %s\n", SDL_GetError());
        return 0;
    }

    // load the wav file here
    SDL_AudioSpec wavSpec;
    if (SDL_LoadWAV("B24PH1004_B24MT1046_B24PH1008_B24CI1041_B24PH1020_Background.wav", &wavSpec, &audio_buffer, &audio_length) == NULL) {
        printf("Could not open audio file: %s\n", SDL_GetError());
        return 0;
    }

    // Open audio device
    device_id = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    if (!device_id) {
        printf("Failed to open audio: %s\n", SDL_GetError());
        SDL_FreeWAV(audio_buffer);
        return 0;
    }
     
    SDL_QueueAudio(device_id, audio_buffer, audio_length);
    
    SDL_PauseAudioDevice(device_id, 0);

    return 1;
}

void check_audio_loop() {
    if (SDL_GetQueuedAudioSize(device_id) == 0) {  
        SDL_QueueAudio(device_id, audio_buffer, audio_length);  
    }
}


void closeAudio() {
    SDL_CloseAudioDevice(device_id);
    SDL_FreeWAV(audio_buffer);
}


void closeSDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void game_run() {
     read_top_scores();
    SDL_Event e;
    while (gameover==0) {
        while (SDL_PollEvent(&e)) {
            input(e);
        }
        update_game();
       render_game();
       check_audio_loop();
        SDL_Delay(16);
    }
}
 
 int SDL_main(int argc, char** argv) {
    
     // Initialize audio first
     if (!audio_error()) {
         return 1;
     }

     // Initialize window second
    if (!error()) {
        return 1;
    }
     
     
    while (1) {
       reset_game();      
        gameover = 0;
        game_run();           

        //  condition for life line of mario
        if (life > 1) {
            life--;
          
        }
        else {
            printf("\n======= GAME OVER! ======\n");
            printf("\nYour Score: %d\n", score);
            printf("\n=========================\n");

            update_top_scores(score);
            print_top_scores();
           
            break;
             
        }
    }
    closeAudio();
    closeSDL();
    SDL_DestroyTexture(bg_texture);
 
    return 0;
}
