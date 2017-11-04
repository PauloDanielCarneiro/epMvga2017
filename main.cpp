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
/*
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
*/
//##################################################################//

////////////////////////////////////////////////////////////////////////
int type = 3;
//CASO 1 EXECUTA CRUST
//CASO 2 EXECUTA BETA-SKELETON
//CASO 3 EXECUTA ARVORE
////////////////////////////////////////////////////////////////////////
//[EP] ----VARIAVEL QUE CONTROLA O ID DO TRIANGULO INICIAL DE BUSCA----/
int id_init = 0;

//[EP] ----REALIZA BUSCA EXAUSTIVA PARA IDENTIFICAR DEFINIR O TRIANGULO DE INICIO----/
//Obs.: Poderia ser feita mudan�a na estrutura de Cell para incluir um campo visitada
//e utilizar o algoritmo mais eficiente do EP
void getInicio(bool clique_direito)
{
     if (clique_direito)
     {
        int i;
	    for (i = 0; i < malha->getNumberOfCells(); i++)
	    {
            double xp = Interactor->getPXD(); //coordenada x
            double yp = Interactor->getPYD(); //coordenada y
            double bar1, bar2, bar3; //coordenadas baricentricas
            bar1 = bar2 = bar3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente
            double xa, ya, xb, yb, xc, yc; //coordenadas dos pontos do triangulo ABC
            //OBTER AS COORDENADAS DOS PONTOS QUE FORMAM UM TRIANGULO----//
            xa = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(0);
            ya = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(1);
            xb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(0);
            yb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(1);
            xc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(0);
            yc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(1);
            //CALCULAR AS AREAS DOS TRIANGULOS
            double areaABC = 0.5*((xa*yb)-(ya*xb)+(ya*xc)-(xa*yc)+(xb*yc)-(yb*xc));
            double areaPBC = 0.5*((xp*yb)-(yp*xb)+(yp*xc)-(xp*yc)+(xb*yc)-(yb*xc));
            double areaAPC = 0.5*((xa*yp)-(ya*xp)+(ya*xc)-(xa*yc)+(xp*yc)-(yp*xc));
            double areaABP = 0.5*((xa*yb)-(ya*xb)+(ya*xp)-(xa*yp)+(xb*yp)-(yb*xp));
            //DETERMINAR AS COORDENDAS BARICENTRICAS
            bar1 = areaPBC/areaABC;
            bar2 = areaAPC/areaABC;
            bar3 = areaABP/areaABC;
            
            //ATUALIZA E ENCERRA SE ENCONTRAR TRIANGULO NO PONTO CLICADO
            if (bar1 > 0 && bar2 > 0 && bar3 > 0)
            {
               id_init = i;
               return;
            }
            //CLIQUE FORA DO MAPA
            else 
            {
                 id_init = 0;
            }
        }
     }
}

//[EP] ----METODO QUE RESOLVE O PROBLEMA PROPOSTO-----------------------------//
void EP(){
     //----OBTER PONTOS DO CLIQUE DO MOUSE----/
    double xp, yp; //coordenadas do ponto P
    xp = Interactor->getPX();
    yp = Interactor->getPY();
    
    //----DEFINE O TRIANGULO INICIAL----//
    int id = id_init;
    
    //NAO IMPRIMIR SE ESTIVER NO ESTADO INICIAL
    if (xp != 0.0 && yp != 0.0)
    {
           
       //[EP] ----COORDENADAS BARICENTRICAS-----------------------------------//
       /*
       Fatos:
            p = bar1*a + bar2*b + bar3*c
            bar1 + bar2 + bar3 = 1
       Contruir um sistema de 3 equa��es e 3 inc�gnitas:
               bar1*xa + bar2*xb + bar3*xc = xp
               bar1*ya + bar2*yb + bar3*yc = yp
               bar1    + bar2    + bar3    = 1
       Os valores dos determinantes podem ser obtidos pela regra de Cramer, e sao 
       equivalentes as seguintes areas dos triangulos:
         Det = areaABC <-> D
         bar1 = areaPBC/areaABC <-> D1/D
         bar2 = areaAPC/areaABC <-> D2/D
         bar3 = areaABP/areaABC <-> D3/D
       Evidentemente a soma D1+D2+D3 = D.

       C�lculos das �reas dos triangulos
                areaABC = 0.5*((xa*yb)-(ya*xb)+(ya*xc)-(xa*yc)+(xb*yc)-(yb*xc))
                areaPBC = 0.5*((xp*yb)-(yp*xb)+(yp*xc)-(xp*yc)+(xb*yc)-(yb*xc))
                areaAPC = 0.5*((xa*yp)-(ya*xp)+(ya*xc)-(xa*yc)+(xp*yc)-(yp*xc))
                areaABP = 0.5*((xa*yb)-(ya*xb)+(ya*xp)-(xa*yp)+(xb*yp)-(yb*xp))
       */
       //---------------------------------------------------------------------//
    
       double bar1, bar2, bar3; //coordenadas baricentricas
       bar1 = bar2 = bar3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente
       double xa, ya, xb, yb, xc, yc; //coordenadas dos pontos do triangulo ABC

       while (bar1 <= 0 || bar2 <= 0 || bar3 <= 0)
       {
          //IMPRIMIR O TRIANGULO VISITADO
          Print->Face(malha->getCell(id), dgreen);
          //----Impress�o o caminho no terminal----/
          //cout<< id<<";";
          
          //OBTER AS COORDENADAS DOS PONTOS QUE FORMAM UM TRIANGULO----//
          xa = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(0);
          ya = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(1);
          xb = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(0);
          yb = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(1);
          xc = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(0);
          yc = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(1);

          //CALCULAR AS AREAS DOS TRIANGULOS
          double areaABC = 0.5*((xa*yb)-(ya*xb)+(ya*xc)-(xa*yc)+(xb*yc)-(yb*xc));
          double areaPBC = 0.5*((xp*yb)-(yp*xb)+(yp*xc)-(xp*yc)+(xb*yc)-(yb*xc));
          double areaAPC = 0.5*((xa*yp)-(ya*xp)+(ya*xc)-(xa*yc)+(xp*yc)-(yp*xc));
          double areaABP = 0.5*((xa*yb)-(ya*xb)+(ya*xp)-(xa*yp)+(xb*yp)-(yb*xp));

          //DETERMINAR AS COORDENDAS BARICENTRICAS
          bar1 = areaPBC/areaABC;
          bar2 = areaAPC/areaABC;
          bar3 = areaABP/areaABC;


          //[EP] ----VERIFICAR SE O PONTO PERTENCE AO TRIANGULO----//
          /*
            Definicao:
            Se as coordenadas baricentricas forem todas positivas, o ponto pertence ao triangulo ABC:
               if (bar1 > 0 && bar2 > 0 && bar3 > 0) { return true; }
            Sen�o, o ponto est� na dire��o oposta � coordenada mais negativa.
            Ou seja, primeiramente devemos verificar qual das coordenadas � a mais negativa:
               if (bar1 < bar2 && bar1 < bar3) { menor = bar1; }
               if (bar2 < bar1 && bar2 < bar3) { menor = bar2; }
               if (bar3 < bar1 && bar3 < bar2) { menor = bar3; }
          */
    
          //ENCERRAR SE CHEGOU AO TRIANGULO DESEJADO
          int prox, menor;          
          //Conforme a defini��o, se as 3 coordenadas s�o positivas ponto esta no triangulo
          if (bar1 > 0 && bar2 > 0 && bar3 > 0)
          {
             //cout<<endl;
             break;
          }
          //NAO CHEGOU AO TRIANGULO, DETERMINAR A MENOR COORDENADA
          else
          {
              //menor: BC = 0, AC = 1, AB =2
              if (bar1 < bar2 && bar1 < bar3)
              {
                 menor = 0; //p encontra-se ao lado da aresta BC
                 prox = malha->getCell(id)->getMateId(0);
              }
              if (bar2 < bar1 && bar2 < bar3)
              {
                 menor = 1; //p encontra-se ao lado da aresta AC
                 prox = malha->getCell(id)->getMateId(1);
              }
              if (bar3 < bar1 && bar3 < bar2)
              {
                 menor = 2; //p encontra-se ao lado da aresta AB
                 prox = malha->getCell(id)->getMateId(2);
              }
              
              //TODO: PERGUNTAR AO PROFESSOR SE PRECISA PERCORRER UM CAMINHO NO MAPA OU SOMENTE O MAIS CURTO COMO NESSE CASO
              
              //VERIFICAR SE CHEGOU NA FRONTEIRA
              if (prox == -1)
              {
                 //IMPRIME A ARESTA POR ONDE SAIU - 3 CASOS
                 if (menor == 0) //BC
                    //IMPRIMIR A ARESTA DE SAIDA
                    Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(1)), malha->getVertex(malha->getCell(id)->getVertexId(2)), black, 3.0);
                 if (menor == 1) //AC
                    //IMPRIMIR A ARESTA DE SAIDA
                    Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(0)), malha->getVertex(malha->getCell(id)->getVertexId(2)), black, 3.0);
                 if (menor == 2) //AB
                    //IMPRIMIR A ARESTA DE DAIDA
                    Print->Edge(malha->getVertex(malha->getCell(id)->getVertexId(0)), malha->getVertex(malha->getCell(id)->getVertexId(1)), black, 3.0);
                 //[EP] ----IMPRESSAO DE TESTE----/
                 //cout<< "Clicou fora do mapa" <<endl;
                 break;
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
    Print->Face(malha->getCell(id_init), yellow);


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
		case 's':
			
			
		break;

		case 'd':
			//RESERVADO PARA O M�TODO QUE VAI ALTERAR O ID DO TRIANGULO INICIAL
			
		break;
	

	}
    
	Interactor->Refresh_List();
	glutPostRedisplay();

}
using namespace std;

int main(int *argc, char **argv)
{

  ofRuppert2D<MyofDefault2D> ruppert;
  ofPoints2DReader<MyofDefault2D> reader;
  ofVtkWriter<MyofDefault2D> writer;
  Interactor->setDraw(RenderScene);
	meshHandler.Set(new TMesh());
      char *fileBrasil = "/home/helton/Downloads/ep_mvga/epMvga2017/Brasil.off";

     
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
        iv->setCoord(1, -iv->getCoord(1)); //DESINVERTE O MAPA DO BRASIL
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

	maxdim *= 0.4;
	
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
