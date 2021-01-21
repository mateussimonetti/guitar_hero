// TP - PDS1
//Aluno: Mateus D Simonetti
//NOTA: em algumas funções, escrevi o comentario resumindo o que a linha faz. Lê-los avulsamente não fará muito sentido, mas ler todos os comentários da função em ordem poderá ser mais explicativo

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdlib.h>
#include <math.h>

#define MAXTILES 5
#define AVG_TILES_PER_LANE 20
#define MAXLANES 6
#define MIN_SCORE -10*NUM_LANES
#define MAX_TAM 1000

#define CREATED 0
#define HIT -1
#define ACTIVE 1
#define LIMBO -2

FILE *arq, *record;




const int FPS = 480;  //com esse FPS não dá pra ver muito bem o fogo e o clique sem interferir na jogabilidade, mas a musica fica mais coerente. Para ver o fogo, 180 de fps e interessante
const int SCREEN_W = 960;
const int SCREEN_H = 640;


int DROP_Y = 225;
int SKY_H;
int TILE_HIT_H;
int TILE_R = 50;
int flag_last_tile = 0;
int flag_fail = 0;
int flag_force_scape = 0;
int music_opt = 0;

int LANE_W;
int NUM_LANES= 5;

int TRACK_W;
int TRACK_LEFT_X;
int TRACK_RIGHT_X;
int KEY1 = ALLEGRO_KEY_1;

int score = 0;
int bar = 0;

ALLEGRO_AUDIO_STREAM *musica = NULL;
ALLEGRO_SAMPLE *fail = NULL, *fon = NULL, *win = NULL; //funçoes de audio
ALLEGRO_SAMPLE_INSTANCE *sampleInstance = NULL;



typedef struct Tile {
	float x,y; //x center, y bottom 
	int lane; 
	int status;
	ALLEGRO_COLOR color;
} Tile;




void init_track() {
	SKY_H = SCREEN_H * 0.25;
	LANE_W = (2*TILE_R) * 1.5;
	TRACK_W = NUM_LANES * LANE_W;
	TRACK_LEFT_X = SCREEN_W/2 - TRACK_W/2;
	TRACK_RIGHT_X = TRACK_LEFT_X + TRACK_W;
}

void draw_scenario(ALLEGRO_DISPLAY *display) {
	
	int i;

	//screen
	ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0); 
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(BKG_COLOR);   
	
	//sky
	al_draw_filled_rectangle(0, 0, SCREEN_W, SKY_H, al_map_rgb(100,100,100));

   //desenha a barra de vida
   al_draw_filled_rectangle(SCREEN_W/2 - 94, 30, SCREEN_W/2 + 94, 60, al_map_rgb(0,0,0)); //fundo preto
   al_draw_filled_rectangle(SCREEN_W/2 - 92, 32, SCREEN_W/2 - 32, 58, al_map_rgb(0,255,0)); // 1/3 verde
   al_draw_filled_rectangle(SCREEN_W/2 - 30, 32, SCREEN_W/2 + 30, 58, al_map_rgb(255,255,0));// 1/3 amarelo
   al_draw_filled_rectangle(SCREEN_W/2 + 32, 32, SCREEN_W/2 + 92, 58, al_map_rgb(255,0,0));// 1/3 vermelho
   
   //desenha a pista:
   al_draw_line(TRACK_LEFT_X, SKY_H, TRACK_LEFT_X, SCREEN_H, al_map_rgb(255,255,255), 10); 
   al_draw_line(TRACK_LEFT_X+(LANE_W*0.5), SKY_H, TRACK_LEFT_X+(LANE_W*1.5) - LANE_W, SCREEN_H, al_map_rgb(255,100,0), LANE_W - NUM_LANES); //(eu sei, ficou estranho, mas coloriu a lane de laranja como eu queria) 
   al_draw_filled_circle(TRACK_LEFT_X+(LANE_W*0.5), 550, 65, al_map_rgb(0,0,0)); //desenha o circulo inferior na primeira lane
   al_draw_line(TRACK_RIGHT_X, SKY_H, TRACK_RIGHT_X, SCREEN_H, al_map_rgb(255,255,255), 10);

   for(i=1; i<NUM_LANES; i++) {
	   al_draw_line(TRACK_LEFT_X+(LANE_W*1.5)+(LANE_W * (i-1)), SKY_H, TRACK_LEFT_X+(LANE_W*1.5)+(LANE_W * (i-1)), SCREEN_H, al_map_rgb(255,100,0), LANE_W - NUM_LANES); //desenha as outras lanes
	   al_draw_line(TRACK_LEFT_X+(LANE_W*i), SKY_H, TRACK_LEFT_X+(LANE_W*i), SCREEN_H, al_map_rgb(255,255,255), NUM_LANES); //(eu usei as variaveis muito mal nessa hora, mas estava aprendendo ainda)
	   al_draw_filled_circle(TRACK_LEFT_X+(LANE_W*1.5)+(LANE_W * (i-1)), 550, 65, al_map_rgb(0,0,0));
	   
   }
   al_draw_circle(TRACK_LEFT_X+(LANE_W*0.5), 550, 55, al_map_rgb(33,94,33), 5.0); //desenha os aneis coloridos em cada lane
   al_draw_circle(TRACK_LEFT_X+(LANE_W*1.5), 550, 55, al_map_rgb(255,0,0), 5.0);
   al_draw_circle(TRACK_LEFT_X+(LANE_W*2.5), 550, 55, al_map_rgb(255,255,0), 5.0);
   al_draw_circle(TRACK_LEFT_X+(LANE_W*3.5), 550, 55, al_map_rgb(0,0,255), 5.0);
   al_draw_circle(TRACK_LEFT_X+(LANE_W*4.5), 550, 55, al_map_rgb(255,127,0), 5.0);
}

 void zera_tiles(Tile *NEW_TILE) {
	NEW_TILE->status = 0;

}

   int check_end(Tile *TILE) {
	  if(flag_last_tile == 1 && TILE->y >= 658) {
		  al_play_sample(fon, 1.0, 0, 1, ALLEGRO_PLAYMODE_ONCE,NULL);
		  al_rest(2);
		  //caso a ultima tile tenha sido criada e esta já tenha passado de 659 no eixo vertical (esteja quase abaixo do visivel pelo cursor)
		  score--; //perde-se o ponto desta ultima tile
		  return 1; //retorna que o programa deve acabar
		} 
      return 0;
}

 void create_tile(Tile *NEW_TILE, int pista,int id, ALLEGRO_COLOR TILE_COLOR) { //cria 
	if (NEW_TILE->status != 1 && id != 0) { 
	NEW_TILE->lane = pista;
	NEW_TILE->x = TRACK_LEFT_X+(LANE_W*1.5)+(LANE_W * (pista - 1));
	NEW_TILE->y = 215;
	NEW_TILE->status = 1;
	NEW_TILE->color = TILE_COLOR;
	}
	if (id == 0) {//Quando já não possuirem mais tiles escritas no txt (o numero 0 na coluna do id das tiles significa isso)	
	flag_last_tile = 1; //a variavel informa que a ultima tile foi criada. Essa tile estara na lane TILE_OPT[MUSIC[numLinhas][0]] e tera o id  MUSIC[numLinhas][1]
	}
}

 void draw_tiles(Tile *NEW_TILE) {
	 if (NEW_TILE->status == 1 ) { //desenha a tecla caso ela tenha sido criada
		 NEW_TILE->y = NEW_TILE->y + 1;
		 al_draw_filled_circle(NEW_TILE->x, NEW_TILE->y, 55, NEW_TILE->color);
		 al_draw_filled_circle(NEW_TILE->x, NEW_TILE->y, 25, al_map_rgb(255,255,255));
		 //al_play_sample(fon, 1.0, 0, 1, ALLEGRO_PLAYMODE_ONCE,NULL);
	 }
 }
 
 void check_tiles(Tile *NEW_TILE) { //checa se a tile passada por referencia está ativa e fora do range de clique

	 if (NEW_TILE->status == 1 && NEW_TILE->y > 660) { //caso ela exista e o eixo y dela já tenha passado do range de clique (esteja quase saindo do display)
        NEW_TILE->status = 0; //zera o status dela, para que possa ser criada outra bolinha com esse id
		al_play_sample(fon, 1.0, 0, 1, ALLEGRO_PLAYMODE_ONCE,NULL);
		score--; //subtrai o score
		bar--; //reduz 1 na barra de vida
		
	 }
 }

int check_position(Tile *TILE) { //checa se a tile (passada por referencia) esta dentro do range de clique
	if (TILE->status ==1 && TILE->y >= 490 && TILE->y <= 580) {	//caso ela exista (status = 1) e esteja no range
		TILE->status = 0; //ela e zerada para que essa tile possa ser criada novamente
		return 1; //informa condição afirmativa.
	}
return 0; //caso ela não exista ou nao esteja no range, retorna condição negativa
}


int bar_control(int bar) {	
	al_draw_line(SCREEN_W/2 - 4*bar, 30, SCREEN_W/2 - 4*bar, 60, al_map_rgb(255,255,255), 5);//desenha a barra com posição relativa ao score
	return bar;
}

void abreMusica(void) {
	do {
		printf("----------------------------------------GUITAR HERO-------------------------------------------------------------");
		printf("\nEscolha a musica que deseja tocar:");
		printf("\n[0] Aquecimento para inicio\n");
		printf("\n[1] Clube dos Canalhas - Matanza (MEDIO/DIFICIL)\n");
		scanf("%d", &music_opt);
	} while (music_opt != 0 && music_opt != 1 );
	switch (music_opt) {
		case 0:
			arq = fopen("aquecimento.txt","r");
			break;
		case 1:
			arq = fopen("musica.txt","r");
			break;
	}
	return;	
}
void escolheMusica(void) {
	switch (music_opt) {
		case 0:
			musica = al_load_audio_stream("Parabéns pra você - Baixo.ogg", 4, 1024);
			break;
		case 1:
			musica = al_load_audio_stream("ClubedosCanalhasEdited.ogg", 4, 1024);
			break;
	}
	//musica = al_load_audio_stream("ClubedosCanalhasEdited.ogg", 4, 1024);
	return;
}

int main(int argc, char *argv[]){
	abreMusica();
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
	//arq = fopen("musica.txt","r");
	if (arq == NULL) {
	printf("erro ao abrir arquivo de musica");
	}
	
	int i, n=0, random=0, j, endgame, score_aux;
    Tile TILE_OPT[51]; //vator de tiles. O valor entre colchetes é o valor do id da tile
	char my_score[10]; 
	int MUSIC[MAX_TAM][4]; //matriz que ira armazenar as informações presentes no txt que controla o nascimento das tiles
	int numLinhas, numColunas, x=0, y=0, aux_x=0; //variaveis usadas so para a matriz (preferi assim para não misturar com i e j)
	for (i=0; i<numLinhas;i++) {
		for (j=0;j<numColunas;j++) {
			MUSIC[i][j] = 0; //algoritmo que limpa a matriz
		}
	}
	fscanf(arq, "%d %d",&numLinhas, &numColunas);
	for (i=0; i<numLinhas;i++) {
		for (j=0;j<numColunas;j++) {
			fscanf(arq, "%d", &MUSIC[i][j]); //algoritmo que passa as informações do txt para a matriz
		}
	}
	fclose(arq);
	for (j=0; j<51; j++)
		zera_tiles(&TILE_OPT[j]); //zera as informações do vetor de tiles

	
	
 
	//----------------------- rotinas de inicializacao ---------------------------------------
   if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }
   
    if(!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
        return -1;
    }
 
   timer = al_create_timer(1.0 / FPS);
   if(!timer) {
      fprintf(stderr, "failed to create timer!\n");
      return -1;
   }
 
   display = al_create_display(SCREEN_W, SCREEN_H);
   if(!display) {
      fprintf(stderr, "failed to create display!\n");
      al_destroy_timer(timer);
      return -1;
   }


	//inicializa o modulo allegro que carrega as fontes
    al_init_font_addon();
	//inicializa o modulo allegro que entende arquivos tff de fontes
    al_init_ttf_addon();
	//inicializa os modulos de som
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(3);
	fail = al_load_sample("FAIL.wav");
	fon = al_load_sample("FON.wav");
	win = al_load_sample("WIN.wav");
	//musica = al_load_audio_stream("ClubedosCanalhasEdited.ogg", 4, 1024);


	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
    ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);   	

	event_queue = al_create_event_queue();
   if(!event_queue) {
      fprintf(stderr, "failed to create event_queue!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }
   
   al_install_keyboard();
   
	//registra na fila de eventos que eu quero identificar quando a tela foi alterada
   al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila de eventos que eu quero identificar quando o tempo alterou de t para t+1
   al_register_event_source(event_queue, al_get_timer_event_source(timer));
   
   al_register_event_source(event_queue, al_get_keyboard_event_source());   
   
  

	//reinicializa a tela
   al_flip_display();
	//inicia o temporizadorp
   al_start_timer(timer);

	init_track();

	escolheMusica();
   int playing = 1;
	//enquanto playing for verdadeiro, faca:
   while(playing) {

	  al_attach_audio_stream_to_mixer(musica, al_get_default_mixer());
	  al_set_audio_stream_playing(musica, true);
	  ALLEGRO_EVENT ev;
	  //espera por um evento e o armazena na variavel de evento ev
      al_wait_for_event(event_queue, &ev);
	  
	 if(ev.type == ALLEGRO_EVENT_KEY_DOWN) { //Caso uma tecla tenha sido pressionada
		int id_min, id_max;
		float lane_mult;
		if (ev.keyboard.keycode == 1) { //avalia qual tecla foi
			id_min = 1; //e a partir disso escolhe o range minimo
			id_max = 11; //e o maximo
			lane_mult = 0.5; //e o multiplicador da lane, necessario para desenhar o "foguinho"
			//printf("tecla da lane verde");
			al_draw_filled_circle(TRACK_LEFT_X+(LANE_W*lane_mult), 550, 55, al_map_rgb(33,94,33));
			}

		
		else if (ev.keyboard.keycode == 19) {
			id_min = 11;
			id_max = 21;
			lane_mult = 1.5;
			al_draw_filled_circle(TRACK_LEFT_X+(LANE_W*lane_mult), 550, 55, al_map_rgb(255,0,0));			
			//printf("\ntecla da lane vermelha");
		}	

		
		else if (ev.keyboard.keycode == 10) {
			id_min = 21;
			id_max = 31;
			lane_mult = 2.5;
			//printf("\ntecla da lane amarela");
						al_draw_filled_circle(TRACK_LEFT_X+(LANE_W*lane_mult), 550, 55, al_map_rgb(255,255,0));
		}

		
		else if (ev.keyboard.keycode == 11) {
			id_min = 31;
			id_max = 41;
			lane_mult = 3.5;
			//printf("\ntecla da lane azul");
			al_draw_filled_circle(TRACK_LEFT_X+(LANE_W*lane_mult), 550, 55, al_map_rgb(0,0,255));
		}
		
		else if (ev.keyboard.keycode == 12) {
			id_min = 41;
			id_max = 51;
			lane_mult = 4.5;
			//printf("\ntecla da lane laranja");
			al_draw_filled_circle(TRACK_LEFT_X+(LANE_W*lane_mult), 550, 55, al_map_rgb(255,127,0));
		 }

		if (ev.keyboard.keycode == 1 ||ev.keyboard.keycode == 19 ||ev.keyboard.keycode == 10 ||ev.keyboard.keycode == 11 ||ev.keyboard.keycode == 12 ) {
			for (j=id_min; j<id_max; j++) { //utilizando o range definido acima
				 if ( check_position(&TILE_OPT[j]) ){ //a função verifica se algum dos ids equivalentes àquela lane está na posição devida para o clic ser valido
					score = score + 2; //caso seja, soma-se 1 ponto (observe que aqui soma 2, mas no final da função subtraimos 1)
				 if ((bar + 1) < 24) //controla a barra de vida, de forma que, caso ela não tenha chegado no seu limite
					bar += 2; //somaremos 1 a ela (observe que aqui soma 2, mas no final da função subtraimos 1)
				 if ((bar + 1) == 24) //caso ela ja esteja cheia
					bar++;	//não acrescentamos pontos a ela
				
				 al_draw_filled_triangle( TRACK_LEFT_X+(LANE_W*lane_mult) - 55, 550, TRACK_LEFT_X+(LANE_W*lane_mult) + 55, 550, TRACK_LEFT_X+(LANE_W*lane_mult), 450, al_map_rgb(255, 0, 0)); //desenha o fogo na lane correspondente
				 al_draw_filled_triangle( TRACK_LEFT_X+(LANE_W*lane_mult) - 45, 550, TRACK_LEFT_X+(LANE_W*lane_mult) + 45, 550, TRACK_LEFT_X+(LANE_W*lane_mult), 510, al_map_rgb(255, 255, 0));
                //printf ("%d / %d\n\n",j, MUSIC[numLinhas - 1][1]);				
				if (flag_last_tile == 1 && j == MUSIC[numLinhas - 2][1]) {// se a ultima tecla ja tiver sido impressa e ela tiver sido apertada.
					bar--;
					//score--;
					al_rest(2);
					playing = 0;
					}
				}
			}
		 bar--;
		 score--;
	}

		if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
			flag_force_scape = 1;
			playing = 0;
		}
		al_flip_display();	
		al_rest(0.02);
	 }
   
	//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
    else if(ev.type == ALLEGRO_EVENT_TIMER) {
		ALLEGRO_COLOR TILE_COLOR =  al_map_rgb(0,0,0);
		draw_scenario(display);
		if (MUSIC[x][2] == 0) {  // a variavel n não pode ser zerada, porque ela "quebra" todas as funções contempladas pelo if. Por isso, quando o arquivo acabar
			MUSIC[aux_x][2] = 100; // um valor qualquer diferente de 0 pode ser utilizadp
		}
		if ((n+1) % MUSIC[aux_x][2] == 0) { //n é uma variavel que controla o tempo de nascimento das tiles. Na matriz, a terceira coluna e destinada a informar o gap entre as tiles
		switch (MUSIC[x][0]) //avalia a primeira informação da linha: qual será a lane que a tile estará
		{
			
			case 0:
			TILE_COLOR = al_map_rgb(33,94,33); //caso seja a primeira lane, a tile é verde
				break;
				
			case 1:
			TILE_COLOR = al_map_rgb(255,0,0);//caso seja a segunda lane, a tile é vermelha
				break;
				
			case 2:
			TILE_COLOR = al_map_rgb(255,255,0);//caso seja a terceira lane, a tile é amarela
				break;
				
			case 3:
			TILE_COLOR = al_map_rgb(0,0,255);//caso seja a quarta lane, a tile é azul
				break;
				
			case 4:
			TILE_COLOR = al_map_rgb(255,127,0); //caso seja a quinta lane, a tile é laranja
				break;
		}

		if (aux_x < numLinhas) {
		create_tile(&TILE_OPT[MUSIC[x][1]], MUSIC[x][0], MUSIC[x][1], TILE_COLOR); //cria a tile que a linha da matriz indica para ser criada, utilizando o id informado pela segunda coluna da linha da matriz
		x++;
		aux_x++;		//dropa para a proxima linha
	    n = 0;    //é importante zerar n pois o resto da divisao deste numero pelo parametro passado pela matriz que sera quem ditará o nascimento das tiles. Ou seja, quando uma tile e gerada, o "contador n" reinicia. 
		}
	}
		n++; //atualiza a variavel de controle de nascimento
		
		
		for (j = 0;j<51;j++) //varre todo o vetor e desenha as teclas que foram criadas
		{	
			
			draw_tiles(&TILE_OPT[j]);
		}
		
		
		//SCORE
		endgame = bar_control(bar); 
		sprintf(my_score, "%d", score);

		 if (endgame == -23) {//caso o jogador erre até o fim da barra vermelha, o jogo acaba 
			 //al_rest(1);
			playing = 0;
			flag_fail = 1;
		 }
		
        al_draw_text(size_32, al_map_rgb(255, 0, 0), SCREEN_W-100, 20, 0, my_score);
		
		//reinicializo a tela
		 al_flip_display();
         for (j=0; j<51; j++)
		{	
			check_tiles(&TILE_OPT[j]); //checa o posicionamento das tiles 
		}
		//printf("\n%d", MUSIC[numLinhas - 2][1] );
	 if ( check_end(&TILE_OPT[MUSIC[numLinhas - 2][1]]) ) { //checa a ultima linha de informação da matriz para descobrir se o jogo acabou ////
	 playing = 0;
	}	 

    }
	//printf("\n%d", MUSIC[numLinhas][1]);

	//se o tipo de evento for o fechamento da tela (clique no x da janela)
	else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
		flag_force_scape = 1;
		playing = 0;
	}
	
	


  } //fim do while
     al_destroy_audio_stream(musica);
	 //al_destroy_sample_instance(sampleInstance);
	//al_uninstall_audio();
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	
	
	al_rest(1);
	
	char my_text[50];
	int recorde;
	record = fopen("recorde.txt","r");
	fscanf(record,"%d",&recorde);
	fclose(record);
	if (score > recorde) {
		al_clear_to_color(al_map_rgb(0,0,0));
		sprintf(my_text, "Novo Recorde!");
		al_draw_text(size_32, al_map_rgb(0, 255, 0), SCREEN_W/3, SCREEN_H/3, 0, my_text);
		al_flip_display();	
		al_rest(1);
		record = fopen("recorde.txt","w");
		fprintf(record, "%d", score);
		recorde = score;
		fclose(record);
	}

	//colore toda a tela de preto
	al_clear_to_color(al_map_rgb(0,0,0));
	if (flag_fail == 1) {
		sprintf(my_text, "VOCE PERDEU!");
		al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/3, 0, my_text);
		al_play_sample(fail, 3.0, 0, 1, ALLEGRO_PLAYMODE_ONCE,NULL);
	}
	else if ( flag_fail == 0 && flag_force_scape == 0){ 
		sprintf(my_text, "PARABENS! VOCE VENCEU!");
		al_draw_text(size_32, al_map_rgb(0, 255, 0), SCREEN_W/3, SCREEN_H/3, 0, my_text);
		al_play_sample(win, 1.0, 0, 1, ALLEGRO_PLAYMODE_ONCE,NULL);		
	}
	sprintf(my_text, "Score: %d", score);
	al_draw_text(size_32, al_map_rgb(200, 0, 30), SCREEN_W/3, SCREEN_H/2, 0, my_text);
	sprintf(my_text, "Record: %d", recorde);
	al_draw_text(size_32, al_map_rgb(0, 255, 0), SCREEN_W/3, SCREEN_H/1.5, 0, my_text);
	//reinicializa a tela
	al_flip_display();	
    if (flag_force_scape == 0)
		al_rest(10);	
	al_rest(1);
  	al_destroy_sample_instance(sampleInstance);
	al_destroy_sample(fail);
	al_destroy_sample(fon);
	al_uninstall_audio();
   al_destroy_timer(timer);
   al_destroy_display(display);
   al_destroy_event_queue(event_queue);

   return 0;
}

