#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <unistd.h>
#include <math.h>
#include <glib.h>

#define WIN_WIDTH 880
#define WIN_HEIGHT 280
#define PLAYER_SIZE 100
#define PLAYER_JUMP_Y WIN_HEIGHT-PLAYER_SIZE*2.5
#define PLAYER_NO_JUMP_Y WIN_HEIGHT-PLAYER_SIZE

#define OBJ_WIDTH 30
#define OBJ_HEIGHT 60

GtkWidget *window;
GtkWidget *fixed;
GtkWidget *g_image_player;
GdkPixbufAnimation* anim_pixbuf;
const GdkPixbuf *src_pixbuf;
GdkPixbuf *dest_pixbuf;
GtkWidget *g_image_obj;
GtkWidget *label_km;
GtkWidget *label_game_over;

int dino_run=1;
int player_x;
int player_y;
int jump_speed;

int can_jump;
int jump_up;
int jump_down;

int obj_x;
int obj_y;
int obj_speed;

int game_state;
int run_km;

void css(GtkWidget *widget, const gchar *data) {
	GtkCssProvider *provider = gtk_css_provider_new ();
	GtkStyleContext *context = gtk_widget_get_style_context(widget);
	gtk_css_provider_load_from_data(provider, data, -1, NULL);
	gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	g_object_unref (provider);
}

void game_init(){
    
    player_x=10;
    player_y=PLAYER_NO_JUMP_Y;
    can_jump=1;
    jump_up=0;
    jump_down=0;
    jump_speed=2;

    obj_speed=2;
    obj_x=WIN_WIDTH;
    obj_y=WIN_HEIGHT-OBJ_HEIGHT;

    game_state=1;
    run_km=0;
    gtk_widget_hide(label_game_over);
}
 
void destroy(GtkWidget *widget,gpointer data){
    gtk_main_quit();
}

void play_audio_jump(){
    //非阻塞调用命令播放声音
    GError *error;
    g_spawn_command_line_async ("play jump.ogg",error);
}
void play_audio_over(){
    //非阻塞调用命令播放声音 游戏结束
    GError *error;
    g_spawn_command_line_async ("play over.ogg",error);
}
void play_audio_ok(){
    //非阻塞调用命令播放声音
    GError *error;
    g_spawn_command_line_async ("play ok.ogg",error);
}

void load_default_img()
{
    src_pixbuf = gdk_pixbuf_new_from_file("img.png", NULL); 
    dest_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, WIN_WIDTH, WIN_HEIGHT, GDK_INTERP_HYPER);
    gtk_image_set_from_pixbuf (GTK_IMAGE(g_image_player),dest_pixbuf);
}

gboolean km(){
    if(game_state==1){
        run_km+=1;
        if(run_km%100==0){
            obj_speed+=1;
            jump_speed+=1;
            play_audio_ok();
        }
        char title[100];
	    sprintf(title, "%d km", run_km);
        gtk_label_set_text(GTK_LABEL(label_km),title);
    }
    return TRUE;
}
gboolean loop(){
    //游戏中
    if(game_state==1){
        //player
        if(jump_up==1){
            player_y-=jump_speed;
            if(player_y<=PLAYER_JUMP_Y){
                jump_up=0;
                jump_down=1;
            }
        }else if(jump_down==1){
            player_y+=jump_speed;
            if(player_y>=PLAYER_NO_JUMP_Y){
                can_jump=1;
                jump_down=0;
                dino_run=1;
            }
        }
        gtk_fixed_move(GTK_FIXED(fixed),g_image_player,player_x,player_y);

        //obj
        obj_x-=obj_speed;
        if(obj_x<=0){
            obj_x=WIN_WIDTH;
        }
        if(abs(obj_x-player_x)<(int)(PLAYER_SIZE+OBJ_WIDTH)/3){
            if(abs(obj_y-player_y)<(int)(PLAYER_SIZE+OBJ_HEIGHT)/3){
                game_state=3;
                play_audio_over();
            } 
        }
        gtk_fixed_move(GTK_FIXED(fixed),g_image_obj,obj_x,obj_y);
    }else if(game_state==2){//暂停
    }else if(game_state==3){//游戏结束
        printf("game over\n");
        gtk_widget_show(label_game_over);
    }
    
    return TRUE;
}

gboolean dino_run_image(){
    if(game_state==1){
        if(dino_run==1){
            src_pixbuf = gdk_pixbuf_new_from_file("dino1.png", NULL); 
            dino_run=2;
        }else if(dino_run==2){
            src_pixbuf = gdk_pixbuf_new_from_file("dino2.png", NULL); 
            dino_run=1;
        }
        dest_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, PLAYER_SIZE, PLAYER_SIZE, GDK_INTERP_HYPER);
        gtk_image_set_from_pixbuf (GTK_IMAGE(g_image_player),dest_pixbuf);
    }
    return TRUE;
}

void dino_jump_image(){
    dino_run=0;
    src_pixbuf = gdk_pixbuf_new_from_file("dino3.png", NULL);
    dest_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, PLAYER_SIZE, PLAYER_SIZE, GDK_INTERP_HYPER);
    gtk_image_set_from_pixbuf (GTK_IMAGE(g_image_player),dest_pixbuf);
}

void obj_image(){
    src_pixbuf = gdk_pixbuf_new_from_file("obj.png", NULL);
    dest_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, OBJ_WIDTH, OBJ_HEIGHT, GDK_INTERP_HYPER);
    gtk_image_set_from_pixbuf (GTK_IMAGE(g_image_obj),dest_pixbuf);
}

gboolean deal_key_press(GtkWidget *widget, GdkEventKey  *event, gpointer data)  
{  
    int key = event->keyval;
    switch(key){
        case 32:
            if(can_jump==1 && game_state==1){
                play_audio_jump();
                jump_up=1;
                jump_down=0;
                can_jump=0;
                dino_jump_image();
                printf("jump\n");  
            }
            if(game_state==3){
                game_init(); 
            }
            break;
    }
//    printf("keyval = %d\n", key);  
    return TRUE;  
}

int main(int argc,char *argv[]){
    gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_set_default_size(GTK_WINDOW(window),WIN_WIDTH,WIN_HEIGHT);
    gtk_window_set_resizable (GTK_WINDOW(window),FALSE);
    g_signal_connect(window, "destroy",G_CALLBACK (destroy), NULL);
    g_signal_connect(window, "key-press-event",G_CALLBACK(deal_key_press), NULL);  
    gtk_window_set_title(GTK_WINDOW(window),"dino game");
    gtk_window_set_keep_above(GTK_WINDOW (window), TRUE);
    fixed = gtk_fixed_new ();

    //km
    label_km = gtk_label_new_with_mnemonic ("");
	gtk_widget_set_size_request(label_km,500,40);
    gtk_misc_set_alignment(GTK_MISC(label_km),0,1);
    css (label_km, "label{color:#222;font-size:30px}");
    gtk_fixed_put (GTK_FIXED(fixed),label_km,10,0);

    //gameover
    label_game_over = gtk_label_new_with_mnemonic ("GAME OVER");
	gtk_widget_set_size_request(label_game_over,WIN_WIDTH,WIN_HEIGHT);
//    gtk_misc_set_alignment(GTK_MISC(label_game_over),1,1);
    css (label_game_over, "label{color:#222;font-size:60px}");
    gtk_fixed_put (GTK_FIXED(fixed),label_game_over,0,0);

    //player
    g_image_player = gtk_image_new ();
    gtk_widget_set_size_request(GTK_WIDGET(g_image_player),PLAYER_SIZE,PLAYER_SIZE);
    gtk_fixed_put (GTK_FIXED(fixed),g_image_player,player_x,player_y);
    gtk_container_add (GTK_CONTAINER (window), fixed);

    //obj
    g_image_obj = gtk_image_new ();
    gtk_widget_set_size_request(GTK_WIDGET(g_image_obj),OBJ_WIDTH,OBJ_HEIGHT);
    gtk_fixed_put (GTK_FIXED(fixed),g_image_obj,obj_x,obj_y);

    gtk_widget_show_all (window);
    gtk_widget_hide(label_game_over);
//    gtk_widget_set_opacity (GTK_WIDGET (window), 0.9);
    
    game_init();
    obj_image();
    g_timeout_add(200, (GSourceFunc)dino_run_image, NULL);
    g_timeout_add(10, (GSourceFunc)loop, NULL);
    g_timeout_add(100, (GSourceFunc)km, NULL);
    printf("%d:%d\n",player_x,player_y);

    gtk_main ();
    return 0;
}


