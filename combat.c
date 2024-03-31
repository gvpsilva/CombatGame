#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <math.h>
//---Constantes ----
const float FPS = 100;
const int SCREEN_W = 1200;
const int SCREEN_H = 700;
const float THETA = M_PI/4;			//angulo theta do triangulo circuscrito
const float RAIO_CAMPO_FORCA =50; //RAIO DO CIRCULLO
const float VEL_TANQUE =2.5;//velocidade do tanque
const float PASSO_ANGULO=M_PI/90;//velocidade 
const int 	NUM_BALAS= 1; // numero de balas por vez na tela
//-- Structs----
typedef struct Ponto{
	float x,y;
} Ponto;
typedef struct Retangulo{
	Ponto sup_esq,inf_dir;
	ALLEGRO_COLOR cor;
}Retangulo;
typedef struct Tanque{
	Ponto centro;
	ALLEGRO_COLOR cor;
	Ponto A, B, C;
	float vel;
	float angulo;
	float x_comp, y_comp;
	float vel_angular;
	int pontuacao;
} Tanque;
typedef struct Bala{
	Ponto p;
	float velocidade;
	bool ativo;
	float x_comp;
	float y_comp;
	int raio;
}Bala;
//---Funções-----
//Rotociona as coordenadas
void Rotate (Ponto *P, float Angle){
	float x=P->x, y=P->y;
	P->x=(x*cos(Angle))-(y*sin(Angle));
	P->y=(y*cos(Angle))+(x*sin(Angle));
}
//Rotaciona o tanque
void rotacionaTanque(Tanque *t){
	if(t->vel_angular!=0){
		Rotate(&t->A,t->vel_angular);
		Rotate(&t->B,t->vel_angular);
		Rotate(&t->C,t->vel_angular);
		t->angulo+=t->vel_angular;
		t->x_comp=cos(t->angulo);
		t->y_comp=sin(t->angulo);
	}
}
//calcula a distancia entre um ponto e uma cordenada x,y
float distanciaEntrePontos(Ponto p1, float x2, float y2){
	return sqrt(pow((p1.x-x2),2)+pow((p1.y-y2),2));
}
//calcula distancia entre dois pontos x,y
float distanciaEntrePontos2(float x1,float y1, float x2, float y2){
	return sqrt(pow((x1-x2),2)+pow((y1-y2),2));
}
//verifica se ha colisão entre tanques, olha a distancia entre 2 vezes o raio
int colisaoEntreTanques(Tanque *t1, Tanque *t2){
	if(distanciaEntrePontos2(t1->centro.x,t1->centro.y, t2->centro.x, t2->centro.y)<=(RAIO_CAMPO_FORCA*2)){
		return 1;
	
	}
}
//verifica se ha colisao entre o circulo e o retangulo central(obtaculo)
int colisaoCirculoRetangulo(Retangulo *r, Tanque *t){
	//regiao 1 e 2
	if(t->centro.x>=r->sup_esq.x && t->centro.x<=r->inf_dir.x){
		if(t->centro.y+RAIO_CAMPO_FORCA>= r->sup_esq.y && t->centro.y-RAIO_CAMPO_FORCA<=r->inf_dir.y){
				return 1;
		}
	}
	//regiao 3 e 4
	if(t->centro.y>=r->sup_esq.y && t->centro.y<=r->inf_dir.y){
		if(t->centro.x+RAIO_CAMPO_FORCA>=r->sup_esq.x && t->centro.x-RAIO_CAMPO_FORCA<=r->inf_dir.x){
				return 2;
		}
	}
	//regiao 5 a 8
		if(	distanciaEntrePontos(t->centro,r->sup_esq.x,r->sup_esq.y)<=RAIO_CAMPO_FORCA||
				distanciaEntrePontos(t->centro,r->sup_esq.x,r->inf_dir.y)<=RAIO_CAMPO_FORCA||
				distanciaEntrePontos(t->centro,r->inf_dir.x,r->sup_esq.y)<=RAIO_CAMPO_FORCA||
				distanciaEntrePontos(t->centro,r->inf_dir.x,r->inf_dir.y)<=RAIO_CAMPO_FORCA){
					return 3;
		}
	return 0;
}
//verifica se há colisao da bala com  o retangulo
int colisaoentreBalaRetangulo(Retangulo *r, Bala *p){
	//regiao 1 e 2
	if(p->p.x>=r->sup_esq.x && p->p.x<=r->inf_dir.x){
		if(p->p.y+p->raio>= r->sup_esq.y && p->p.y-p->raio<=r->inf_dir.y){
				return 1;
		}
	}
	//regiao 3 e 4
	if(p->p.y>=r->sup_esq.y && p->p.y<=r->inf_dir.y){
		if(p->p.x+p->raio<= r->sup_esq.x && p->p.x-p->raio>=r->inf_dir.x){
				return 1;
		}
	}
	//regiao 5 a 8
		if(	distanciaEntrePontos2(p->p.x,p->p.y,r->sup_esq.x,r->sup_esq.y)<=p->raio||
				distanciaEntrePontos2(p->p.x,p->p.y,r->sup_esq.y,r->inf_dir.x)<=p->raio||
				distanciaEntrePontos2(p->p.x,p->p.y,r->sup_esq.x,r->inf_dir.y)<=p->raio||
				distanciaEntrePontos2(p->p.x,p->p.y,r->inf_dir.x,r->inf_dir.y)<=p->raio){
					return 1;
		}
	return 0;
}
//verifica se ha colisão bala com o tanquer, ou seja se um tanque antigiu o outro
int colisaoBalaTanque(Bala *p, Tanque *t){
	if(distanciaEntrePontos2(p->p.x,p->p.y,t->centro.x,t->centro.y)<=(RAIO_CAMPO_FORCA+10)){
		return 1;
	}
	return 0;
}
//verifica se ha colisao entre tanque e tela
int colisaoTanqueTela(Tanque *t){
	//lado esquerdo
		if(t->centro.x+RAIO_CAMPO_FORCA>=SCREEN_W){
					return 1;			
		}
			if(t->centro.x-RAIO_CAMPO_FORCA<=0){
					return 1;			
		}
			if(t->centro.y+RAIO_CAMPO_FORCA>=SCREEN_H){
					return 1;			
		}
			if(t->centro.y-RAIO_CAMPO_FORCA<=0){
					return 1;			
		}
}
//Atualiza o desenho do tanque na tela
void atualizaTanque(Tanque *t){
				rotacionaTanque(t);				
				t->centro.y+=t->vel*t->y_comp;
				t->centro.x+=t->vel*t->x_comp;

	
}
//inicializa o tanque 1
void initTanque(Tanque *t){
	t->centro.x=(SCREEN_W)/6;
	t->centro.y=(SCREEN_H/2)-200;
	t->cor=al_map_rgb(6, 82, 221);
	float alpha = (M_PI/2)-THETA;
	float h= RAIO_CAMPO_FORCA*  sin(alpha);
	float w = RAIO_CAMPO_FORCA* sin(THETA);
	t->A.x=0;
	t->A.y=-RAIO_CAMPO_FORCA;
	t->B.x=-w;
	t->B.y=h;
	t->C.x=w;
	t->C.y=h;
	t->vel=0;
	t->angulo=M_PI/2;
	t->x_comp=cos(t->angulo);
	t->y_comp=sin(t->angulo);
	t->vel_angular=0;
	t->pontuacao=0;
}
//inicializa o tanque 2
void initTanque2(Tanque *t){
	t->centro.x=(SCREEN_W/6)*5;
	t->centro.y=(SCREEN_H/2)+200;
	t->cor=al_map_rgb(234, 32, 39);
	float alpha = (M_PI/2)-THETA;
	float h= RAIO_CAMPO_FORCA*  sin(alpha);
	float w = RAIO_CAMPO_FORCA* sin(THETA);
	t->A.x=0;
	t->A.y=-RAIO_CAMPO_FORCA;
	t->B.x=-w;
	t->B.y=h;
	t->C.x=w;
	t->C.y=h;
	t->vel=0;
	t->angulo=M_PI/2;
	t->x_comp=cos(t->angulo);
	t->y_comp=sin(t->angulo);
	t->vel_angular=0;
	t->pontuacao=0;
}
//desenha o tanque 
void desenhaTanque(Tanque t){
	al_draw_circle(t.centro.x, t.centro.y,RAIO_CAMPO_FORCA, t.cor, 3);
	al_draw_filled_triangle(t.A.x+t.centro.x, t.A.y+t.centro.y,
													t.B.x+t.centro.x, t.B.y+t.centro.y,
													t.C.x+t.centro.x, t.C.y+t.centro.y,
													t.cor);
}
//inicializa o retangulo central da tela
void initRetangulo(Retangulo *r){
		r->sup_esq.x=450;
		r->sup_esq.y=(SCREEN_H/2)-100;
		r->inf_dir.x=750;
		r->inf_dir.y=(SCREEN_H/2)+100;
		r->cor = al_map_rgb(0, 148, 50);
}
//desenha o retangulo na tela
void desenhaRetangulo(Retangulo r){
	al_draw_filled_rectangle(r.sup_esq.x,r.sup_esq.y,r.inf_dir.x,	r.inf_dir.y,r.cor);
}
//desenha o cenário 
void desenhaCenario(){
	al_clear_to_color(al_map_rgb(34, 47, 62));
}
//inicializa as balas dos tanques
void initBala(Bala balas[],int tam){
	int i=0;
	for(i=0;i<tam;i++){
		balas[i].ativo=false;
		balas[i].velocidade=8;
		balas[i].raio=10;
	}
}
//atira as balas
void atiraBalas(Bala balas[], int tam,Tanque t){
	int i;
	for(i=0;i<tam;i++){
		if(!balas[i].ativo){
			
			balas[i].p.x=t.centro.x+t.A.x;
		  balas[i].p.y=t.centro.y+t.A.y;
			balas[i].x_comp=t.x_comp;
			balas[i].y_comp=t.y_comp;
			balas[i].ativo=true;
			break;
		}
	}
}
//atualiza a  bala na tela sem alterar a trajetoria
void atualizaBalas(Bala balas[], int tam,Tanque *t1, Tanque *t2,Retangulo *r){
	int i;
	for(i=0;i<tam;i++){	
		
			balas[i].p.x-=(balas[i].velocidade*balas[i].x_comp);
			balas[i].p.y-=(balas[i].velocidade*balas[i].y_comp);
			if(balas[i].p.x>SCREEN_W){
				balas[i].ativo=false;
			}
			if(balas[i].p.x<0){
					balas[i].ativo=false;
			}
			if(balas[i].p.y<0){
					balas[i].ativo=false;
			}
			if(balas[i].p.y>SCREEN_H){
					balas[i].ativo=false;
			}
			
			if(colisaoBalaTanque(&balas[i],t2)==1){
					balas[i].ativo=false;
					t1->pontuacao++;
			}
			if(colisaoentreBalaRetangulo(r,&balas[i])==1){
					balas[i].ativo=false;
			}
			
		}
	}
//desenha as balas na tela
void desenhaBalas(Bala balas[],int tam){
		int i;
	for(i=0;i<tam;i++){
		if(balas[i].ativo){
			al_draw_filled_circle(balas[i].p.x, balas[i].p.y,balas[i].raio, al_map_rgb(255,255,255));
		}

	}
}
//desenha o cenario final, exibi placar e historico de partida
int desenhaCenarioFinal(Tanque t1,Tanque t2){
	FILE *fp;
  char c;
	int texto[3];
  int n = 0;
	fp=fopen ("Historico.txt","r");  
	while (fscanf(fp, "%d", &texto[n]) == 1){
			n++;
	} 
	   fclose(fp);
		 	if(t1.pontuacao==5){
				texto[0]++;
		}
		else{
				texto[1]++;
		}	
	
		fp=fopen ("Historico.txt","w");  
	 	fprintf(fp, "%d\n",texto[0]);
		fprintf(fp, "%d\n",texto[1]);
		fclose(fp);
		ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 34, 1);
	  	al_clear_to_color(al_map_rgb(255, 255, 255));
		al_draw_textf(size_32,al_map_rgb(0, 0, 0),SCREEN_W/2,(SCREEN_H-SCREEN_H/2),1,"PONTUAÇÃO");
		al_draw_textf(size_32,al_map_rgb(0, 0, 0),SCREEN_W/2,(SCREEN_H-SCREEN_H/2+34),1,"%d  X %d",t1.pontuacao,t2.pontuacao);
		al_draw_textf(size_32,al_map_rgb(0, 0, 0),SCREEN_W/2,(SCREEN_H-SCREEN_H/3+34),1,"Histórico de Pontuação");
		al_draw_textf(size_32,al_map_rgb(0, 0, 0),SCREEN_W/2,(SCREEN_H-SCREEN_H/3+34*2),1,"Jogador 1:%i",texto[0]);
		al_draw_textf(size_32,al_map_rgb(0, 0, 0),SCREEN_W/2,(SCREEN_H-SCREEN_H/3+34*3),1,"Jogador 2:%i",texto[1]);
	
		return -1;
}

//---MAIN-----
int main(int argc, char **argv){
//variaveis e inicializaçoes
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_FONT *fonte =NULL;
	bool game_over=false;	

	//----------------------- rotinas de inicializacao ---------------------------------------

	//inicializa o Allegro
	if (!al_init()){
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}
	//inicializa o módulo de primitivas do Allegro
	if (!al_init_primitives_addon()){
		fprintf(stderr, "failed to initialize primitives!\n");
		return -1;
	}
	//inicializa o modulo que permite carregar imagens no jogo
	if (!al_init_image_addon()){
		fprintf(stderr, "failed to initialize image module!\n");
		return -1;
	}
	//cria um temporizador que incrementa uma unidade a cada 1.0/FPS segundos
	timer = al_create_timer(1.0 / FPS);
	if (!timer){
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}
	//cria uma tela com dimensoes de SCREEN_W, SCREEN_H pixels
	display = al_create_display(SCREEN_W, SCREEN_H);
	if (!display){
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}
  //instala o teclado
	if (!al_install_keyboard()){
		fprintf(stderr, "failed to install keyboard!\n");
		return -1;
	}
	//inicializa o modulo allegro que carrega as fontes
	al_init_font_addon();
	al_init_ttf_addon();
	//inicializa o modulo allegro que entende arquivos tff de fontes
	if (!al_init_ttf_addon()){
		fprintf(stderr, "failed to load tff font module!\n");
		return -1;
	}
	//carrega o arquivo arial.ttf da fonte Arial e define que sera usado o tamanho 32 (segundo parametro)
	ALLEGRO_FONT *size_32 = al_load_font("arial.ttf", 32, 1);
	if (size_32 == NULL){
		fprintf(stderr, "font file does not exist or cannot be accessed!\n");
	}
	//cria a fila de eventos
	event_queue = al_create_event_queue();
	if (!event_queue){
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		return -1;
	}
	//registra na fila os eventos de tela (ex: clicar no X na janela)
	al_register_event_source(event_queue, al_get_display_event_source(display));
	//registra na fila os eventos de tempo: quando o tempo altera de t para t+1
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	//registra na fila os eventos de teclado (ex: pressionar uma tecla)
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	//inicia o temporizador
	al_start_timer(timer);
	int playing = 1;
	Tanque tanque_1;//cria o tanque 1
	initTanque(&tanque_1);//inicializa o tanque 1
	Tanque tanque_2;//cria o tanque 2
	initTanque2(&tanque_2);//inicializa o tanque 1
	Retangulo r1;//cria o retangulo 
	initRetangulo(&r1);//inicializa o retangulo
	Bala bala1[NUM_BALAS];//instancia as balas do tanque 1
	initBala(bala1,NUM_BALAS);//inicializa as balas do tanque 1
	Bala bala2[NUM_BALAS];//cria as balas para o tanque 2
	initBala(bala2,NUM_BALAS);//inicializa as balas do tanque 2
 //durante a execursao do jogo
	while (playing)
	{
		ALLEGRO_EVENT ev;
		//espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);
		//se o tipo de evento for um evento do temporizador, ou seja, se o tempo passou de t para t+1
		if (ev.type == ALLEGRO_EVENT_TIMER){
			if(!game_over){
				desenhaCenario();
				desenhaRetangulo(r1);		
				atualizaTanque(&tanque_1);	
				atualizaTanque(&tanque_2);
				desenhaTanque(tanque_1);				
				desenhaTanque(tanque_2);
				//verifica se o tanque 1 atirou
				if(bala1[0].ativo==true){
					atualizaBalas(bala1,NUM_BALAS,&tanque_1,&tanque_2,&r1);	
					//verifica se acabou jogo
					if (tanque_1.pontuacao==5)
					{
						game_over=true;
						desenhaCenarioFinal(tanque_1,tanque_2);
					}		
					desenhaBalas(bala1,NUM_BALAS);
				}
				//verifica se o tanque atirou
				if(bala2[0].ativo==true){
					atualizaBalas(bala2,NUM_BALAS,&tanque_2,&tanque_1,&r1);
					//verifica pontuação jogador 2
					if (tanque_2.pontuacao==5)
					{
						game_over=true;
						desenhaCenarioFinal(tanque_1,tanque_2);
					
					}	
					desenhaBalas(bala2,NUM_BALAS);
				}
				//atualiza o placar na tela
				if(game_over==false){
					al_draw_textf(size_32,al_map_rgb(255,255,255),100,0,1,"Jogador 1: %d", tanque_1.pontuacao);
				  al_draw_textf(size_32,al_map_rgb(255,255,255),SCREEN_W-100,0,1,"Jogador 2: %d", tanque_2.pontuacao);
				}
			  //verifica colisoes					
				if(colisaoEntreTanques(&tanque_1,&tanque_2)==1){
						tanque_1.vel=0;
				}
				if(colisaoEntreTanques(&tanque_2,&tanque_1)==1){
				
						tanque_2.vel=0;
				}
				if(colisaoTanqueTela(&tanque_1)==1){
					tanque_1.vel=0;
				}
				if(colisaoTanqueTela(&tanque_2)==1){
					tanque_2.vel=0;
				}
				if(colisaoCirculoRetangulo(&r1,&tanque_1)!=0){
					
							tanque_1.vel=0;
				}	
				if(colisaoCirculoRetangulo(&r1,&tanque_2)!=0){
						tanque_2.vel=0;
				}
				//atualiza a tela (quando houver algo para mostrar)
				al_flip_display();
				if (al_get_timer_count(timer) % (int)FPS == 0)
					printf("\n%d segundos se passaram...", (int)(al_get_timer_count(timer) / FPS));
				}
			}
		//se o tipo de evento for o fechamento da tela (clique no x da janela)
		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
			playing = 0;
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			int temp;
			//imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);
				switch (ev.keyboard.keycode){	
					//teclas de movimentação		
					case ALLEGRO_KEY_W:{	
						//verifica se houve colisao	com a tela
						if(colisaoTanqueTela(&tanque_1)==1){
							//verifica se a direção 
								if(tanque_1.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_1.centro.x-RAIO_CAMPO_FORCA<=0 
									||tanque_1.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_1.centro.y-RAIO_CAMPO_FORCA<=0 ){	
										//tela parte superior
											if(tanque_1.centro.y+tanque_1.A.y>=tanque_1.centro.y
															&& (tanque_1.centro.y==0+RAIO_CAMPO_FORCA || tanque_1.centro.y<=RAIO_CAMPO_FORCA)){
																tanque_1.vel-=VEL_TANQUE;
																break;	
												}
										//tela parte inferior
													if(tanque_1.centro.y+tanque_1.A.y<=SCREEN_H-RAIO_CAMPO_FORCA &&  (tanque_1.centro.y==SCREEN_H-RAIO_CAMPO_FORCA||tanque_1.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H)){
																tanque_1.vel-=VEL_TANQUE;
																break;	
												}
										//tela lado esquerdo
											if(tanque_1.centro.x+tanque_1.A.x>=tanque_1.centro.x
															&& (tanque_1.centro.x==0+RAIO_CAMPO_FORCA || tanque_1.centro.x<=RAIO_CAMPO_FORCA)){
																tanque_1.vel-=VEL_TANQUE;
																break;	
												}
										//tela lado direito
											if(tanque_1.centro.x+tanque_1.A.x<=SCREEN_W-RAIO_CAMPO_FORCA &&  (tanque_1.centro.x==SCREEN_W-RAIO_CAMPO_FORCA||tanque_1.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W)){
																tanque_1.vel-=VEL_TANQUE;
																break;	
												}
																		tanque_1.vel=0;
													break;
										}
						}	
						//verifica se houve colisao com o outro tanque
						if(colisaoEntreTanques(&tanque_1,&tanque_2)==1){
											if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																										tanque_2.centro.x,tanque_2.centro.y)>=RAIO_CAMPO_FORCA*2){
															
														
														tanque_1.vel-=VEL_TANQUE;
													break;
										}
										if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																										tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA){
													tanque_1.vel=0;
													break;
										}
											if(distanciaEntrePontos2(tanque_1.centro.x,tanque_1.centro.y,
																										tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA*2){
													tanque_1.vel=0;
													break;
										}
										
									}	
						//Verifica colisão tanque com  retangulo
						//verifica colisão com regiões 1 e 2
							if(colisaoCirculoRetangulo(&r1,&tanque_1)==1){
										if(tanque_1.centro.y+tanque_1.A.y<=r1.sup_esq.y-RAIO_CAMPO_FORCA){
											tanque_1.vel-=VEL_TANQUE;
											break;
										}
										if(tanque_1.centro.y+tanque_1.A.y>=r1.inf_dir.y+RAIO_CAMPO_FORCA){
											tanque_1.vel-=VEL_TANQUE;
											break;
										}
										tanque_1.vel=0;
										break;
								}
						//verifica colisão regiao 3 e 4 
							if(colisaoCirculoRetangulo(&r1,&tanque_1)==2){					
									if(tanque_1.centro.x+tanque_1.A.x<=r1.sup_esq.x-RAIO_CAMPO_FORCA){

												tanque_1.vel-=VEL_TANQUE;
											break;
									}
								if(tanque_1.centro.x+tanque_1.A.x>=r1.inf_dir.x+RAIO_CAMPO_FORCA){
												tanque_1.vel-=VEL_TANQUE;
											break;
									}
										tanque_1.vel=0;
										break;
								}
						//quina dos retangulos
							if(colisaoCirculoRetangulo(&r1,&tanque_1)==3){
									//canto superior esquerdo
									if(tanque_1.centro.x+tanque_1.A.x<=r1.sup_esq.x-RAIO_CAMPO_FORCA || tanque_1.centro.y+tanque_1.A.y<=r1.sup_esq.y-RAIO_CAMPO_FORCA){
												tanque_1.vel-=VEL_TANQUE;
											break;
									}
								if(tanque_1.centro.x+tanque_1.A.x<=r1.sup_esq.x-RAIO_CAMPO_FORCA || tanque_1.centro.y+tanque_1.A.y>=r1.inf_dir.y+RAIO_CAMPO_FORCA){
											tanque_1.vel-=VEL_TANQUE;
											break;
									}
									if(tanque_1.centro.x+tanque_1.A.x>=r1.inf_dir.x+RAIO_CAMPO_FORCA || tanque_1.centro.y+tanque_1.A.y<=r1.sup_esq.y-RAIO_CAMPO_FORCA){
												tanque_1.vel-=VEL_TANQUE;
											break;
									}
										if(tanque_1.centro.x+tanque_1.A.x>=r1.inf_dir.x+RAIO_CAMPO_FORCA || tanque_1.centro.y+tanque_1.A.y>=r1.inf_dir.y+RAIO_CAMPO_FORCA){
												tanque_1.vel-=VEL_TANQUE;
											break;
									}
										tanque_1.vel=0;
										break;
								}
						//nao houve colisao
							tanque_1.vel-=VEL_TANQUE;
								break;
						}
					case ALLEGRO_KEY_UP:{
							//verifica se houve colisao	com a tela
						if(colisaoTanqueTela(&tanque_2)==1){
						//verifica se a direção 
							if(tanque_2.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_2.centro.x-RAIO_CAMPO_FORCA<=0 
								||tanque_2.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_2.centro.y-RAIO_CAMPO_FORCA<=0 ){	
									//tela parte superior
										if(tanque_2.centro.y+tanque_2.A.y>=tanque_2.centro.y
														&& (tanque_2.centro.y==0+RAIO_CAMPO_FORCA || tanque_2.centro.y<=RAIO_CAMPO_FORCA)){
															tanque_2.vel-=VEL_TANQUE;
															break;	
											}
									//tela parte inferior
										if(tanque_2.centro.y+tanque_2.A.y<=SCREEN_H-RAIO_CAMPO_FORCA &&  (tanque_2.centro.y==SCREEN_H-RAIO_CAMPO_FORCA
												||tanque_2.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H)){
															tanque_2.vel-=VEL_TANQUE;
															break;	
											}
									//tela lado esquerdo
										if(tanque_2.centro.x+tanque_2.A.x>=tanque_2.centro.x
														&& (tanque_2.centro.x==0+RAIO_CAMPO_FORCA || tanque_2.centro.x<=RAIO_CAMPO_FORCA)){
															tanque_2.vel-=VEL_TANQUE;
															break;	
											}
									//tela lado direito
										if(tanque_2.centro.x+tanque_2.A.x<=SCREEN_W-RAIO_CAMPO_FORCA &&
											(tanque_2.centro.x==SCREEN_W-RAIO_CAMPO_FORCA||tanque_2.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W)){
															tanque_2.vel-=VEL_TANQUE;
															break;	
											}
																	
										tanque_2.vel=0;
										break;
									}
						}
						//verifica se houve colisao	com outro tanque
						if(colisaoEntreTanques(&tanque_1,&tanque_2)==1){
										if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																									tanque_1.centro.x,tanque_1.centro.y)>=RAIO_CAMPO_FORCA*2){
													tanque_2.vel-=VEL_TANQUE;
												break;
									}
									if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																									tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA){
												tanque_2.vel=0;
												break;
									}
										if(distanciaEntrePontos2(tanque_2.centro.x,tanque_2.centro.y,
																									tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA*2){
												tanque_2.vel=0;
												break;
									}
									
								
								}
						////verifica colisão regiao 1 e 2
						if(colisaoCirculoRetangulo(&r1,&tanque_2)==1){
									if(tanque_2.centro.y+tanque_2.A.y<=r1.sup_esq.y-RAIO_CAMPO_FORCA){
											tanque_2.vel-=VEL_TANQUE;
											break;
										}
									if(tanque_2.centro.y+tanque_2.A.y>=r1.inf_dir.y+RAIO_CAMPO_FORCA){
											tanque_2.vel-=VEL_TANQUE;
											break;
										}
										tanque_2.vel=0;
										break;
									}
						//verifica colisão regiao 3 e 4 
						if(colisaoCirculoRetangulo(&r1,&tanque_2)==2){					
								if(tanque_2.centro.x+tanque_2.A.x<=r1.sup_esq.x-RAIO_CAMPO_FORCA){

											tanque_2.vel-=VEL_TANQUE;
										break;
								}
							if(tanque_2.centro.x+tanque_2.A.x>=r1.inf_dir.x+RAIO_CAMPO_FORCA){
											tanque_2.vel-=VEL_TANQUE;
										break;
								}
									tanque_2.vel=0;
									break;
							}
						//Verifica colisão com  os vertices
						if(colisaoCirculoRetangulo(&r1,&tanque_2)==3){
								//canto superior esquerdo
								if(tanque_2.centro.x+tanque_2.A.x<=r1.sup_esq.x-RAIO_CAMPO_FORCA || tanque_2.centro.y+tanque_2.A.y<=r1.sup_esq.y-RAIO_CAMPO_FORCA){
											tanque_2.vel-=VEL_TANQUE;
										break;
								}
							if(tanque_2.centro.x+tanque_2.A.x<=r1.sup_esq.x-RAIO_CAMPO_FORCA || tanque_2.centro.y+tanque_2.A.y>=r1.inf_dir.y+RAIO_CAMPO_FORCA){
										tanque_2.vel-=VEL_TANQUE;
										break;
								}
								if(tanque_2.centro.x+tanque_2.A.x>=r1.inf_dir.x+RAIO_CAMPO_FORCA || tanque_2.centro.y+tanque_2.A.y<=r1.sup_esq.y-RAIO_CAMPO_FORCA){
											tanque_2.vel-=VEL_TANQUE;
										break;
								}
									if(tanque_2.centro.x+tanque_2.A.x>=r1.inf_dir.x+RAIO_CAMPO_FORCA || tanque_2.centro.y+tanque_2.A.y>=r1.inf_dir.y+RAIO_CAMPO_FORCA){
											tanque_2.vel-=VEL_TANQUE;
										break;
								}
									tanque_2.vel=0;
									break;
							}
						tanque_2.vel-=VEL_TANQUE;
						break;
						}
					case ALLEGRO_KEY_S:	{
						//verifica se houve colisao com a tela
						if(colisaoTanqueTela(&tanque_1)==1){
							//verifica se a direção 
								if(tanque_1.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_1.centro.x-RAIO_CAMPO_FORCA<=0 
									||tanque_1.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_1.centro.y-RAIO_CAMPO_FORCA<=0 ){	
										//tela parte superior
											if(tanque_1.centro.y+tanque_1.A.y<=RAIO_CAMPO_FORCA
															&& (tanque_1.centro.y==0+RAIO_CAMPO_FORCA || tanque_1.centro.y<=SCREEN_H)){
																tanque_1.vel+=VEL_TANQUE;
																break;	
												}
										//tela parte inferior
													if(tanque_1.centro.y+tanque_1.A.y>=tanque_1.centro.y && 
													(tanque_1.centro.y==SCREEN_H-RAIO_CAMPO_FORCA||tanque_1.centro.y>=SCREEN_H-RAIO_CAMPO_FORCA)){
																tanque_1.vel+=VEL_TANQUE;
																break;	
												}
										//tela lado esquerdo
											if(tanque_1.centro.x+tanque_1.A.x<=tanque_1.centro.x
															&& (tanque_1.centro.x==0+RAIO_CAMPO_FORCA || tanque_1.centro.x<=RAIO_CAMPO_FORCA)){
																tanque_1.vel+=VEL_TANQUE;
																break;	
												}
										//tela lado direito
											if(tanque_1.centro.x+tanque_1.A.x>=tanque_1.centro.x &&
												(tanque_1.centro.x==SCREEN_W-RAIO_CAMPO_FORCA||tanque_1.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W)){
																tanque_1.vel+=VEL_TANQUE;
																break;	
												}
																		tanque_1.vel=0;
													break;
										}
						}	
						//verifica colisão com outro tanque	
						if(colisaoEntreTanques(&tanque_1,&tanque_2)==1){
										if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																										tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA*2){
														tanque_1.vel+=VEL_TANQUE;
													break;
										}
										if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																										tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA){
													tanque_1.vel=0;
													break;
										}
											if(distanciaEntrePontos2(tanque_1.centro.x,tanque_1.centro.y,
																										tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA*2){
													tanque_1.vel=0;
													break;
										}
										
									
									}
						
						if(colisaoCirculoRetangulo(&r1,&tanque_1)==1){
							if(tanque_1.centro.y+tanque_1.A.y>=r1.sup_esq.y-RAIO_CAMPO_FORCA && tanque_1.centro.y<=r1.sup_esq.y){
											tanque_1.vel+=VEL_TANQUE;
											break;
							}
							if(tanque_1.centro.y+tanque_1.A.y<=r1.inf_dir.y+RAIO_CAMPO_FORCA && tanque_1.centro.y>=r1.inf_dir.y){
									tanque_1.vel+=VEL_TANQUE;
									break;
							}
										tanque_1.vel=0;
										break;
								}
						//verifica colisão regiao 3 e 4 
						if(colisaoCirculoRetangulo(&r1,&tanque_1)==2){					
									if(tanque_1.centro.x+tanque_1.A.x>=r1.sup_esq.x-RAIO_CAMPO_FORCA && tanque_1.centro.x<=r1.sup_esq.x){
												tanque_1.vel+=VEL_TANQUE;
											break;
									}
								if(tanque_1.centro.x+tanque_1.A.x<=r1.inf_dir.x+RAIO_CAMPO_FORCA && tanque_1.centro.x>=r1.inf_dir.x){
												tanque_1.vel+=VEL_TANQUE;
											break;
									}
										tanque_1.vel=0;
										break;
								}
						//quina dos retangulos
							if(colisaoCirculoRetangulo(&r1,&tanque_1)==3){
							//canto superior esquerdo
					  	if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																				r1.sup_esq.x,r1.sup_esq.y)<=RAIO_CAMPO_FORCA){
										tanque_1.vel+=VEL_TANQUE;
									break;
							}
							if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																				r1.sup_esq.x,r1.inf_dir.y)<=RAIO_CAMPO_FORCA){
										tanque_1.vel+=VEL_TANQUE;
									break;
							}
							if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																				r1.inf_dir.x,r1.sup_esq.y)<=RAIO_CAMPO_FORCA){
										tanque_1.vel+=VEL_TANQUE;
									break;
							}
							if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																				r1.inf_dir.x,r1.inf_dir.y)<=RAIO_CAMPO_FORCA){
										tanque_1.vel+=VEL_TANQUE;
									break;
							}
								tanque_1.vel=0;
								break;
						}

						tanque_1.vel+=VEL_TANQUE;					
						break;
						}
					case ALLEGRO_KEY_DOWN:{
						//verifica se houve colisao com a tela
						if(colisaoTanqueTela(&tanque_2)==1){
							//verifica se a direção 
								if(tanque_2.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_2.centro.x-RAIO_CAMPO_FORCA<=0 
									||tanque_2.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_2.centro.y-RAIO_CAMPO_FORCA<=0 ){	
										//tela parte superior
											if(tanque_2.centro.y+tanque_2.A.y<=RAIO_CAMPO_FORCA
															&& (tanque_2.centro.y==0+RAIO_CAMPO_FORCA || tanque_2.centro.y<=SCREEN_H)){
																tanque_2.vel+=VEL_TANQUE;
																break;	
												}
										//tela parte inferior
													if(tanque_2.centro.y+tanque_2.A.y>=tanque_2.centro.y && 
													(tanque_2.centro.y==SCREEN_H-RAIO_CAMPO_FORCA||tanque_2.centro.y>=SCREEN_H-RAIO_CAMPO_FORCA)){
																tanque_2.vel+=VEL_TANQUE;
																break;	
												}
										//tela lado esquerdo
											if(tanque_2.centro.x+tanque_2.A.x<=tanque_2.centro.x
															&& (tanque_2.centro.x==0+RAIO_CAMPO_FORCA || tanque_2.centro.x<=RAIO_CAMPO_FORCA)){
																tanque_2.vel+=VEL_TANQUE;
																break;	
												}
										//tela lado direito
											if(tanque_2.centro.x+tanque_2.A.x>=tanque_2.centro.x &&
												(tanque_2.centro.x==SCREEN_W-RAIO_CAMPO_FORCA||tanque_2.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W)){
																tanque_2.vel+=VEL_TANQUE;
																break;	
												}
																		tanque_2.vel=0;
													break;
										}
						}			
			
							//colisao entre tanques
						if(colisaoEntreTanques(&tanque_1,&tanque_2)==1|| colisaoTanqueTela(&tanque_1)==1){
											if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																										tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA*2){
														tanque_2.vel+=VEL_TANQUE;
													break;
										}
										if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																										tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA){
													tanque_2.vel=0;
													break;
										}
											if(distanciaEntrePontos2(tanque_2.centro.x,tanque_2.centro.y,
																										tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA*2){
													tanque_2.vel=0;
													break;
										}
										
									
									}
					
							if(colisaoCirculoRetangulo(&r1,&tanque_2)==1){
							if(tanque_2.centro.y+tanque_2.A.y>=r1.sup_esq.y-RAIO_CAMPO_FORCA && tanque_2.centro.y<=r1.sup_esq.y){
											tanque_2.vel+=VEL_TANQUE;
											break;
							}
							if(tanque_2.centro.y+tanque_2.A.y<=r1.inf_dir.y+RAIO_CAMPO_FORCA && tanque_2.centro.y>=r1.inf_dir.y){
									tanque_2.vel+=VEL_TANQUE;
									break;
							}
										tanque_2.vel=0;
										break;
								}
						//verifica colisão regiao 3 e 4 
							if(colisaoCirculoRetangulo(&r1,&tanque_2)==2){					
								if(tanque_2.centro.x+tanque_2.A.x>=r1.sup_esq.x-RAIO_CAMPO_FORCA && tanque_2.centro.x<=r1.sup_esq.x){
												tanque_2.vel+=VEL_TANQUE;
											break;
									}
								if(tanque_2.centro.x+tanque_2.A.x<=r1.inf_dir.x+RAIO_CAMPO_FORCA && tanque_2.centro.x>=r1.inf_dir.x){
												tanque_2.vel+=VEL_TANQUE;
											break;
									}
										tanque_2.vel=0;
										break;
								}
							if(colisaoCirculoRetangulo(&r1,&tanque_2)==3){
									//canto superior esquerdo
									if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																						r1.sup_esq.x,r1.sup_esq.y)<=RAIO_CAMPO_FORCA){
												tanque_2.vel+=VEL_TANQUE;
											break;
									}
									if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																						r1.sup_esq.x,r1.inf_dir.y)<=RAIO_CAMPO_FORCA){
												tanque_2.vel+=VEL_TANQUE;
											break;
									}
									if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																						r1.inf_dir.x,r1.sup_esq.y)<=RAIO_CAMPO_FORCA){
												tanque_2.vel+=VEL_TANQUE;
											break;
									}
									if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																						r1.inf_dir.x,r1.inf_dir.y)<=RAIO_CAMPO_FORCA){
												tanque_2.vel+=VEL_TANQUE;
											break;
									}
										tanque_2.vel=0;
										break;
								}
								tanque_2.vel+=VEL_TANQUE;
							break;
						}
				  //teclas de rotação
					case ALLEGRO_KEY_A:								
							tanque_1.vel_angular-=PASSO_ANGULO;
						break;
					case ALLEGRO_KEY_LEFT:
							tanque_2.vel_angular-=PASSO_ANGULO;
						break;
							case ALLEGRO_KEY_D:						
							tanque_1.vel_angular+=PASSO_ANGULO;
								
						break;
					case ALLEGRO_KEY_RIGHT:
							tanque_2.vel_angular+=PASSO_ANGULO;
						break;
					//teclas para disparo de tiros
						case ALLEGRO_KEY_Q:
							atiraBalas(bala1,NUM_BALAS,tanque_1);
						break;
						case ALLEGRO_KEY_ENTER:
							atiraBalas(bala2,NUM_BALAS,tanque_2);
						break;
						case  ALLEGRO_KEY_O:
						if(	r1.sup_esq.x<=r1.inf_dir.x)
							r1.inf_dir.x=r1.inf_dir.x-(r1.inf_dir.x*0,25);
						if(	r1.sup_esq.y<=r1.inf_dir.y)
							r1.inf_dir.y=r1.inf_dir.y-(r1.inf_dir.y*0,25);
						if(	r1.sup_esq.x<=r1.inf_dir.x)
							r1.sup_esq.x=r1.sup_esq.x+(r1.sup_esq.x*0,25);
						if(	r1.sup_esq.y<=r1.inf_dir.y)
							r1.sup_esq.y=r1.sup_esq.y+(r1.sup_esq.y*0,25);
						break;
				}
		}
		//se o tipo de evento for um pressionar de uma tecla
		else if (ev.type == ALLEGRO_EVENT_KEY_UP){
			//imprime qual tecla foi
			printf("\ncodigo tecla: %d", ev.keyboard.keycode);
			switch (ev.keyboard.keycode){
				//teclas de movimento
				case ALLEGRO_KEY_W:{
					//verifica se houve colisao tanque-tela
						if(colisaoTanqueTela(&tanque_1)==1){
							if(tanque_1.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_1.centro.x-RAIO_CAMPO_FORCA<=0 
									||tanque_1.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_1.centro.y-RAIO_CAMPO_FORCA<=0 ){
											tanque_1.vel=0;													
												break;
									}
						}
					//verifica se houve colisao tanque-tanque
						if(colisaoEntreTanques(&tanque_1,&tanque_2)!=0){
								
									if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																									tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA){
												tanque_1.vel=0;
												break;
									}
										if(distanciaEntrePontos2(tanque_1.centro.x,tanque_1.centro.y,
																									tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA*2){
												tanque_1.vel=0;
												break;
									}
								}
						if(colisaoCirculoRetangulo(&r1,&tanque_1)!=0){
									tanque_1.vel=0;
												break;
						}		
						tanque_1.vel+=VEL_TANQUE;
								
						break;
					}
				case ALLEGRO_KEY_UP:{
					//colisão entre tanque e tela
					if(colisaoTanqueTela(&tanque_2)==1){
						if(tanque_2.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_2.centro.x-RAIO_CAMPO_FORCA<=0 
								||tanque_2.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_2.centro.y-RAIO_CAMPO_FORCA<=0 ){
										tanque_2.vel=0;													
											break;
								}
					}
					//colisao entre tanques
					if(colisaoEntreTanques(&tanque_1,&tanque_2)==1){
								
								if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																								tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA){
											tanque_2.vel=0;
											break;
								}
									if(distanciaEntrePontos2(tanque_2.centro.x,tanque_2.centro.y,
																								tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA*2){
											tanque_2.vel=0;
											break;
								}
								
							
							}
					//colisao entre retangulo e tanque 2
					if(colisaoCirculoRetangulo(&r1,&tanque_2)!=0){
								tanque_2.vel=0;
											break;
					}		
					tanque_2.vel+=VEL_TANQUE;
					break;
					}
				case ALLEGRO_KEY_S:{
					//verifica se houve colisao tanque-tela
						if(colisaoTanqueTela(&tanque_1)==1){
							if(tanque_1.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_1.centro.x-RAIO_CAMPO_FORCA<=0 
									||tanque_1.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_1.centro.y-RAIO_CAMPO_FORCA<=0 ){
											tanque_1.vel=0;													
												break;
									}
						}
					//colisao entre tanques
						if(colisaoEntreTanques(&tanque_1,&tanque_2)==1){
									
									if(distanciaEntrePontos2(tanque_1.centro.x+tanque_1.A.x,tanque_1.centro.y+tanque_1.A.y,
																									tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA){
												tanque_1.vel=0;
												break;
									}
										if(distanciaEntrePontos2(tanque_1.centro.x,tanque_1.centro.y,
																									tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA*2){
												tanque_1.vel=0;
												break;
									}
									
								
								}
						if(colisaoCirculoRetangulo(&r1,&tanque_1)!=0){
									tanque_1.vel=0;
												break;
						}	
						tanque_1.vel-=VEL_TANQUE;

						break;
					}			
				case ALLEGRO_KEY_DOWN:{
					//colisao tanque tela
					if(colisaoTanqueTela(&tanque_2)==1){
							if(tanque_2.centro.x+RAIO_CAMPO_FORCA>=SCREEN_W || tanque_2.centro.x-RAIO_CAMPO_FORCA<=0 
									||tanque_2.centro.y+RAIO_CAMPO_FORCA>=SCREEN_H || tanque_2.centro.y-RAIO_CAMPO_FORCA<=0 ){
											tanque_2.vel=0;													
												break;
									}
						}
					//colisao entre tanques
					if(colisaoEntreTanques(&tanque_1,&tanque_2)==1){
									
									if(distanciaEntrePontos2(tanque_2.centro.x+tanque_2.A.x,tanque_2.centro.y+tanque_2.A.y,
																									tanque_2.centro.x,tanque_2.centro.y)<=RAIO_CAMPO_FORCA){
												tanque_2.vel=0;
												break;
									}
										if(distanciaEntrePontos2(tanque_2.centro.x,tanque_2.centro.y,
																									tanque_1.centro.x,tanque_1.centro.y)<=RAIO_CAMPO_FORCA*2){
												tanque_2.vel=0;
												break;
									}
									
								
								}
					if(colisaoCirculoRetangulo(&r1,&tanque_2)!=0){
							tanque_2.vel=0;
							break;
						}					
							tanque_2.vel-=VEL_TANQUE;
						break;
				}
				case ALLEGRO_KEY_D:					
						tanque_1.vel_angular-=PASSO_ANGULO;
				break;				
					case ALLEGRO_KEY_RIGHT:
						tanque_2.vel_angular-=PASSO_ANGULO;
					break;
				case ALLEGRO_KEY_A:					
						tanque_1.vel_angular+=PASSO_ANGULO;
					break;
				case ALLEGRO_KEY_LEFT:
						tanque_2.vel_angular+=PASSO_ANGULO;
					break;				
			}
		
		}
	} //fim do while
	//procedimentos de fim de jogo (fecha a tela, limpa a memoria, etc)
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	return 0;
}