#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <list>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <time.h>
//#include <functional>

#include "of.h"
#include "ofOffPointsReader.h"
#include "Handler.hpp" 
#include "GL_Interactor.h"
#include "ColorRGBA.hpp"
#include "Cores.h"
#include "Point.hpp"
#include "printof.hpp"


#include "CommandComponent.hpp"
#include "MyCommands.hpp"

#include "ofVertexStarIteratorSurfaceVertex.h"


clock_t start_insert;
clock_t end_insert;
clock_t start_print;
clock_t end_print;



using namespace std;
using namespace of;

//Define o tamanho da tela.
scrInteractor *Interactor = new scrInteractor(800, 600);
//EP
//Define a malha a ser usada.
typedef of::MyofDefault2D TTraits;
typedef of::ofMesh<TTraits> TMesh;
TMesh *malha;
Handler<TMesh> meshHandler;

typedef PrintOf<TTraits> TPrintOf;

TPrintOf *Print;

typedef MyCommands<TPrintOf> TMyCommands;
typedef CommandComponent TAllCommands;

ofVtkWriter<TTraits> writer;
TAllCommands *allCommands;
//##################################################################//

////////////////////////////////////////////////////////////////////////
int type = 3;
//CASO 1 EXECUTA CRUST
//CASO 2 EXECUTA BETA-SKELETON
//CASO 3 EXECUTA ARVORE
////////////////////////////////////////////////////////////////////////

//[EP] ----VARIAVEL QUE CONTROLA O ID DO TRIANGULO INICIAL DE BUSCA----/
int id_atual = 255;
void getInicio(bool clique_direito);
void baricentrico(double& b1, double& b2, double& b3, double& xp, double& yp, int i);
void EP();

//[EP] ----REALIZA BUSCA EXAUSTIVA PARA IDENTIFICAR DEFINIR O TRIANGULO DE INICIO----/
//Obs.: Poderia ser feita mudan�a na estrutura de Cell para incluir um campo visitada
//e utilizar o algoritmo mais eficiente do EP
void getInicio(bool clique_direito)
{
     if (clique_direito)
     {
		int i = 0;
		int celulas = malha->getNumberOfCells();
	    while (i < celulas)
	    {
            double xp = Interactor->getPXD(); //coordenada x
            double yp = Interactor->getPYD(); //coordenada y
            double b1, b2, b3; //coordenadas baricentricas
            b1 = b2 = b3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente
            baricentrico(b1, b2, b3, xp, yp, i);
            //ATUALIZA E ENCERRA SE ENCONTRAR TRIANGULO NO PONTO CLICADO
            if (b1 > 0 && b2 > 0 && b3 > 0)
            {
				id_atual = i;
               	return;
            }
            //CLIQUE FORA DO MAPA
            else 
            {
				id_atual = 255;
			}
		i++;
        }
     }
}

void baricentrico(double& b1, double& b2, double& b3, double& xp, double& yp, int i){
	double xa, ya, xb, yb, xc, yc; //coordenadas dos pontos do triangulo ABC
	//OBTER AS COORDENADAS DOS PONTOS QUE FORMAM UM TRIANGULO----//
	xa = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(0);
	ya = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(1);
	xb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(0);
	yb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(1);
	xc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(0);
	yc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(1);
	//CALCULAR AS AREAS DOS TRIANGULOS
	double ABC = 0.5*((xa*yb)-(ya*xb)+(ya*xc)-(xa*yc)+(xb*yc)-(yb*xc));
	double PBC = 0.5*((xp*yb)-(yp*xb)+(yp*xc)-(xp*yc)+(xb*yc)-(yb*xc));
	double APC = 0.5*((xa*yp)-(ya*xp)+(ya*xc)-(xa*yc)+(xp*yc)-(yp*xc));
	double ABP = 0.5*((xa*yb)-(ya*xb)+(ya*xp)-(xa*yp)+(xb*yp)-(yb*xp));
	//DETERMINAR AS COORDENDAS BARICENTRICAS
	b1 = PBC/ABC;
	b2 = APC/ABC;
	b3 = ABP/ABC;
}

//[EP] ----METODO QUE RESOLVE O PROBLEMA PROPOSTO-----------------------------//
void EP(){
     //----OBTER PONTOS DO CLIQUE DO MOUSE----/
    double xp, yp; //coordenadas do ponto P
    xp = Interactor->getPX();
    yp = Interactor->getPY();
    
    //----DEFINE O TRIANGULO INICIAL----//
    int id = id_atual;
    
    //NAO IMPRIMIR SE ESTIVER NO ESTADO INICIAL
    if (xp != 0.0 && yp != 0.0)
    {    
       double b1, b2, b3; //coordenadas baricentricas
       b1 = b2 = b3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente

       while (b1 <= 0 || b2 <= 0 || b3 <= 0)
       {
          //IMPRIMIR O TRIANGULO VISITADO
          Print->Face(malha->getCell(id), dgreen);
          //----Impress�o o caminho no terminal----/
          //cout<< id<<";";
          
          baricentrico(b1, b2, b3, xp, yp, id);


          //[EP] ----VERIFICAR SE O PONTO PERTENCE AO TRIANGULO----//
          /*
            Definicao:
            Se as coordenadas baricentricas forem todas positivas, o ponto pertence ao triangulo ABC:
               if (bar1 > 0 && bar2 > 0 && bar3 > 0) { return true; }
            Sen�o, o ponto est� na dire��o oposta � coordenada mais negativa.
            Ou seja, primeiramente devemos verificar qual das coordenadas � a mais negativa:
               if (bar1 < bar2 && bar1 < bar3) { CoordMenor = bar1; }
               if (bar2 < bar1 && bar2 < bar3) { CoordMenor = bar2; }
               if (bar3 < bar1 && bar3 < bar2) { CoordMenor = bar3; }
          */
    
          //ENCERRAR SE CHEGOU AO TRIANGULO DESEJADO
          int prox, CoordMenor;          
          //Verifica se coordenadas são positivas
          if (b1 > 0 && b2 > 0 && b3 > 0)
          {
             break;
          }
          //Não é o triangulo procurado, determinar vertice de menor valor
          else
          {
              //CoordMenor: BC = 0, AC = 1, AB =2
              if (b1 < b2 && b1 < b3)
              {
                 prox = malha->getCell(id)->getMateId(0);
                 CoordMenor = 0; //p está saindo de BC
              }
              if (b2 < b1 && b2 < b3)
              {
                 prox = malha->getCell(id)->getMateId(1);
                 CoordMenor = 1; //p está saindo de AC
              }
              if (b3 < b1 && b3 < b2)
              {
				 prox = malha->getCell(id)->getMateId(2);
                 CoordMenor = 2; //p está saindo de AB
              }

              //Verifica se chegou na borda
              if (prox == -1)
              {
				  //IMPRIME A ARESTA POR ONDE SAIU - 3 CASOS
				switch(CoordMenor){
					case 0:
						Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(1)), malha->getVertex(malha->getCell(id)->getVertexId(2)), black, 3.0);
						break;
					case 1:
						Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(0)), malha->getVertex(malha->getCell(id)->getVertexId(2)), black, 3.0);
						break;
					case 2:
						Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(0)), malha->getVertex(malha->getCell(id)->getVertexId(1)), black, 3.0);
						break
				}
              }
          }
          id = prox;
       }
    } 
}




//----[EP] M�TODO QUE DESENHA NA TELA----//
void RenderScene(void){
	allCommands->Execute();
	
	//[EP] OBS: o que aparece primeiro � pintado por cima
	Print->Vertices(malha,blue,3);
	//[EP] IMPRIME A FACE ON VAI INICIAR O PERCURSO
    Print->Face(malha->getCell(id_atual), yellow);


    //[EP] ----METODOS DE IMPRESSAO QUE FORAM UTILIZADOS NO DESENVOLVIMENTO----/
    /*
	/----IMPRIME OS NUMEROS DOS VERTICES E TRIANGULOS----/
    Print->CellsIds(malha, green); //Imprime o ID de todas as c�lulas
	Print->VerticesIds(malha, blue); //Imprime os IDs dos v�rtices
    
    /----EXEMPLOS DE IMPRESSAO DE ARESTAS----/
    Print->Edge(malha->getVertex(0), malha->getVertex(1), blue, 10.0);
    Print->Edge(1, 2, blue, 10.0);
    
	/----METODO QUE PINTA AS FACES DOS TRI�NGULOS DAS FRONTEIRAS----/
	//Obs.: Fronteiras s�o os tri�ngulos cujo MateId de alguma das arestas � igual a -1
	int i;
	for(i = 0; i < malha->getNumberOfCells(); i++)
	{
          if(malha->getCell(i)->getMateId(0) == -1)
          {
              Print->Face(malha->getCell(i), red);
              continue;
          }
          if(malha->getCell(i)->getMateId(1) == -1)
          {
              Print->Face(malha->getCell(i), red);
              continue;
          }
          if(malha->getCell(i)->getMateId(2) == -1)
          {
              Print->Face(malha->getCell(i), red);
              continue;
          }
    }
    /----PEGAR O ID DO VERTICE N DA CELULA #----/
    //Obs.: cada celula/triangulo tem possui os vertices 0, 1 e 2;
    int m = malha->getCell(255)->getVertexId(0); //Peguei O ID do V�rtice 0 da c�lula 255
    /----ROTINA DE IMPRESS�O DAS COORDENADAS A, B, C DE UM TRIANGULO DADO - EX.: 255----/
    int j;
    int k = 0;
    for (j = 0; j < 6; j++)
    {
        k = j/2;
        int d = malha->getCell(255)->getVertexId(k);
        cout<< d <<"," <<ct[j] <<endl;
    }
    */
    //[FIM]
    
    //[EP] ----CHAMA METODO EXAUSTIVO PARA ATUALIZAR TRIANGULO INICIAL----//
    getInicio(Interactor->getMouseRight());
    //[EP] ----IMPRIME CAMINHO POR COORDENADAS BARICENTRICAS----//
    EP();

	Print->FacesWireframe(malha,grey,3);
	glFinish();
	glutSwapBuffers();
}

void HandleKeyboard(unsigned char key, int x, int y){	
	
	
	
	double coords[3];
	char *xs[10];
	allCommands->Keyboard(key);
	
	switch (key) {

		case 'e':
			exit(1);
		break;
		case 'v':
			coords[0]=x;
			coords[1]=-y;
			coords[2]=0.0;
			malha->addVertex(coords);
		break;
	}
    
	Interactor->Refresh_List();
	glutPostRedisplay();

}



/*
int id_atual = 300; // começa no triangulo 1
double pontoX = 0.0; // variavel que armazena o pontoX 
double pontoY = 0.0; // variavel que armazena o pontoY

// Declaração das funções:
/*
1- captura coordenada
2- realiza calculos baricentricos
3- Procura qual o triangulo que foi clicado
4- verifica se o ponto clicado pertence ao triangulo atual
5- Metodo que vai ligar todas as partes anteriores e resolver o problema

void getCoordenates();
void baricentrico(double px, double py);
void CalculateBari(double xp, double yp, int id, double& b1, double& b2, double& b3);
bool perteceAtual(int& CoordMenor, double b1, double b2, double b3);
//Funções
void getCoordenates(){
	double px, py;
	px = Interactor->getPXD();
	py = Interactor->getPYD();
	baricentrico(px, py);
}

void baricentrico(double px, double py){
	double b1, b2, b3;
	int i;
	int cells = malha->getNumberOfCells();//pega o numero de celulas do mapa

	for(i = 0; i < cells; i++){
		b1 = b2 = b3 = -1;//valor inicial antes do calculo
		CalculateBari(px, py, i, b1, b2, b3);

		//procura triangulo clicado
		if(b1 > 0 && b2 > 0 && b3 > 0){
			id_atual = i;
			break;
		}else{//clique fora do mapa reseta a função
			id_atual = 300;
		}
	}

}

void CalculateBari(double xp, double yp, int id, double& b1, double& b2, double& b3){
	/* A formula para calcularas coordenadas baricentricas se baseia no metodo:
	1- Obter as coordenadas dos vértices do triangulo
	2- calcular a area dos triangulos internos com base nas coordenadas dadas 
	3- Realizar a divisão entre os triangulos para gerar as coordenadas baricentricas
	

	double xa, ya, xb, yb, xc, yc;

	//1- Obter so vértices do triangulo
	xa = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(0);
	ya = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(1);
	xb = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(0);
	yb = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(1);
	xc = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(0);
	yc = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(1);

	//2- calcular a area dos triangulos internos com base nas coordenadas dadas 
	double ABC = 0.5*((xa*yb)-(ya*xb)+(ya*xc)-(xa*yc)+(xb*yc)-(yb*xc));
	double PBC = 0.5*((xp*yb)-(yp*xb)+(yp*xc)-(xp*yc)+(xb*yc)-(yb*xc));
	double APC = 0.5*((xa*yp)-(ya*xp)+(ya*xc)-(xa*yc)+(xp*yc)-(yp*xc));
	double ABP = 0.5*((xa*yb)-(ya*xb)+(ya*xp)-(xa*yp)+(xb*yp)-(yb*xp));

	//3- Realizar a divisão entre os triangulos para gerar as coordenadas baricentricas
	b1 = PBC/ABC;
	b2 = APC/ABC;
	b3 = ABP/ABC;
}

bool perteceAtual(int& CoordMenor, double b1, double b2, double b3){
	/*
		Para verificar se 9o ponto pertence ao triangulo, 
		devemos verificar se as coordenadas baricentricas são todas positivas.
		Caso contrario, deverá sair pelo lado oposto ao vertice de CoordMenor valor.
	
	if (b1 > 0 && b2 > 0 && b3 > 0) return true;
	else if (b1 < b2 && b1 < b3) CoordMenor = 0;
	else if (b2 < b1 && b2 < b3) CoordMenor = 1;
	else CoordMenor = 2;
	return false; 
}

//5- Metodo que vai ligar todas as partes anteriores e resolver o problema
void resolucao(double xp, double yp, int id){
	double b1, b2, b3;//coordenadas baricentricas
	double xa, ya, xb, yb, xc, yc;//triangulo ABC
	int prox, CoordMenor;
	int vertice[2];
	bool res;

	b1 = b2 = b3 = -1;

	while(b1 <= 0 || b2 <= 0 || b3 <= 0){
		//imprime o triangulo atual
		Print->Face(malha->getCell(id), dgreen);
		//Calcula as coordenadas
		CalculateBari(xp, yp, id, b1, b2, b3);
		//verifica qual o triangulo desejado
		res = perteceAtual(CoordMenor, b1, b2, b3);
		if (res) break;
		else{
			//pega o valor do proximo triangulo
			prox = malha->getCell(id)->getMateId(CoordMenor);
			//verifica se chegou a fronteira
			if (prox == -1){
				//Mostra por qual vertice ele saiu
				switch(CoordMenor){
					case 0:
						vertice[0] = 1;
						vertice[1] = 2;
						break;
					case 1:
						vertice[0] = 0;
						vertice[1] = 2;
						break;
					case 2:
						vertice[0] = 0;
						vertice[1] = 1;
						break;
				}
				Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(vertice[0])), malha->getVertex(malha->getCell(id)->getVertexId(vertice[1])), black, 3.0);
				break;
			}
		}
		id = prox;
	}
}

void RenderScene(void){ 
	allCommands->Execute();
	Print->Vertices(malha,blue,3);

	//EP
	//Imprime primeiro triangulo
	Print->Face(malha->getCell(id_atual), yellow);

	//Chama metodo para definir inicio
	if(Interactor->getMouseRight()){
		getCoordenates();
		pontoX = Interactor->getPX();
		pontoY = Interactor->getPY();
		Print->FacesWireframe(malha, grey, 3);
		glFinish();
		glutSwapBuffers();
	}

	//Caminho por coordenadas baricentricas
	if(pontoX != Interactor->getPX() || pontoY != Interactor->getPY()){
		resolucao(Interactor->getPX(), Interactor->getPY(), id_atual);
		Print->FacesWireframe(malha, grey, 3);
		glFinish();
		glutSwapBuffers();
	}
	
	Print->FacesWireframe(malha,grey,3);
	glFinish();
	glutSwapBuffers();
}

void HandleKeyboard(unsigned char key, int x, int y){	
	double coords[3];
	char *xs[10];
	allCommands->Keyboard(key);
	
	switch (key) {

		case 'e':
			exit(1);
		break;
		case 'v':
			coords[0]=x;
			coords[1]=-y;
			coords[2]=0.0;
			malha->addVertex(coords);
		break;
		case 's':
			
			
		break;

		case 'd':
			
			
		break;
	

	}
    
	
	Interactor->Refresh_List();
	glutPostRedisplay();

}*/

using namespace std;

int main(int *argc, char **argv)
{

  ofRuppert2D<MyofDefault2D> ruppert;
  ofPoints2DReader<MyofDefault2D> reader;
  ofVtkWriter<MyofDefault2D> writer;
  Interactor->setDraw(RenderScene);
	meshHandler.Set(new TMesh());
      char *fileBrasil = "/home/paulo/Downloads/epMvga2017/Brasil.off";

     
    reader.readOffFile(fileBrasil);
    
    ruppert.execute2D(reader.getLv(),reader.getLids(),true);
    //writer.write(ruppert.getMesh(),"out.vtk",reader.getNorma(),ruppert.getNumberOfInsertedVertices());
  
  meshHandler = ruppert.getMesh();
  malha = ruppert.getMesh();
  
  
  Print = new TPrintOf(meshHandler);

	allCommands = new TMyCommands(Print, Interactor);

	double a,x1,x2,y1,y2,z1,z2; 

	of::ofVerticesIterator<TTraits> iv(&meshHandler);

	iv.initialize();
	x1 = x2 = iv->getCoord(0);
	y1 = y2 = iv->getCoord(1);
	z1 = z2 = iv->getCoord(2);

	for(iv.initialize(); iv.notFinish(); ++iv){
		if(iv->getCoord(0) < x1) x1 = a = iv->getCoord(0);
		if(iv->getCoord(0) > x2) x2 = a = iv->getCoord(0);
		if(iv->getCoord(1) < y1) y1 = a = iv->getCoord(1);
		if(iv->getCoord(1) > y2) y2 = a = iv->getCoord(1);
		if(iv->getCoord(2) < z1) z1 = a = iv->getCoord(2);
		if(iv->getCoord(2) > z2) z2 = a = iv->getCoord(2);
	}

	double maxdim;
	maxdim = fabs(x2 - x1);
	if(maxdim < fabs(y2 - y1)) maxdim = fabs(y2 - y1);
	if(maxdim < fabs(z2 - z1)) maxdim = fabs(z2 - z1);

	maxdim *= 0.6;
	
	Point center((x1+x2)/2.0, (y1+y2)/2.0, (y1+y2)/2.0 );
	Interactor->Init(center[0]-maxdim, center[0]+maxdim,
					center[1]-maxdim, center[1]+maxdim,
					center[2]-maxdim, center[2]+maxdim,argc,argv);

	
	
	AddKeyboard(HandleKeyboard);

	allCommands->Help(std::cout);
	std::cout<< std::endl<< "Press \"?\" key for help"<<std::endl<<std::endl;
	double t;
	
	Init_Interactor();

  
  return EXIT_SUCCESS;
}
