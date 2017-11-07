#include <stdlib.h>
#include <math.h>

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
void inicializarBari(double& b1, double& b2, double& b3)
void getCoordenadas(double& xp, double& yp);
void verificaPositivo(double& b1,double& b2,double& b3, int i);
void baricentrico(double& b1, double& b2, double& b3, double& xp, double& yp, int i);
double[] area(double[6] coordenadas, double xp, double yp);
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
            double xp = 0.0; //coordenada x
            double yp = 0.0; //coordenada y
            double b1, b2, b3; //coordenadas baricentricas
            //b1 = b2 = b3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente
            inicializarBari(b1, b2, b3);
            getCoordenadas(xp, yp);
            baricentro(b1, b2, b3, xp, yp, i);
            verificaPositivo(b1, b2, b3, i);
            //ATUALIZA E ENCERRA SE ENCONTRAR TRIANGULO NO PONTO CLICADO
		i++;
        }
     }
}

void inicializarBari(double& b1, double& b2, double& b3){
    b1 = -1;
    b2 = -1;
    b3 = -1;
}

void getCoordenadas(double& xp, double& yp){
    xp = Interactor->getPXD();
    yp = Interactor->getPYD();
}

void verificaPositivo(double& b1,double& b2,double& b3, int i){
    (b1 > 0 && b2 > 0 && b3 > 0) ? id_atual = i : id_atual = 255;
}


void baricentrico(double& b1, double& b2, double& b3, double& xp, double& yp, int i){
	double coord [6]; //coordenadas dos pontos do triangulo ABC
    //OBTER AS COORDENADAS DOS PONTOS QUE FORMAM UM TRIANGULO----//
    int aux1, aux2, contador;
    contador = 0;
    for(aux1 = 0; aux1 < 2; aux1++){
        for(aux2 = 0; aux2 < 2; aux2++){
            coord[contador] = malha->getVertex(malha->getCell(i)->getVertexId(aux1))->getCoord(0=aux2);
        }
    }
    double pontos[3] = area(coord, xp, yp);
    b1 = pontos[0];
    b2 = pontos[1];
    b3 = pontos[2];
    //DETERMINAR AS COORDENDAS BARICENTRICAS
    
}

double area[3](double[6] coordenadas, double xp, double yp, double& t1, double& t2, double t3, double& t4){
    //CALCULAR AS AREAS DOS TRIANGULOS

    double A = sqrt(abs(coord[0]-coord[2])*abs(coord[1]-coord[3]));
    double C = sqrt(abs(coord[0]-coord[4])*abs(coord[1]-coord[5]));
    double B = sqrt(abs(coord[2]-coord[4])*abs(coord[3]-coord[5]));

    double SA = 0.5 * (-A + B + C);
    double SB = 0.5 * (A - B + C);
    double SC = 0.5 * (C + B - C);


    double baricentros[3] = {(SB*SC), (SC*SA), (SA*SB)};
    return baricentros;
    /*
	double t1 = ((coord[0]*coord[3])-(coord[2]*coord[1])+(coord[2]*coord[4])-(coord[0]*coord[5])+(coord[1]*coord[5])-(coord[3]*coord[4]));
	double t2 = ((xp*coord[3])-(yp*coord[1])+(yp*coord[4])-(xp*coord[5])+(coord[1]*coord[5])-(coord[3]*coord[4]));
	double t3 = ((coord[0]*yp)-(coord[2]*xp)+(coord[2]*coord[4])-(coord[0]*coord[5])+(xp*coord[5])-(yp*coord[4]));
	double t4 = ((coord[0]*coord[3])-(coord[2]*coord[1])+(coord[2]*xp)-(coord[0]*yp)+(coord[1]*yp)-(coord[3]*xp));
	*/
}

//[EP] ----METODO QUE RESOLVE O PROBLEMA PROPOSTO-----------------------------//
void EP(){
     //----OBTER PONTOS DO CLIQUE DO MOUSE----/
    double xp, yp; //coordenadas do ponto P
    getCoordenadas(xp, yp);
    
    //----DEFINE O TRIANGULO INICIAL----//
    int id = id_atual;
    
    //NAO IMPRIMIR SE ESTIVER NO ESTADO INICIAL
    if (xp != 0.0 && yp != 0.0)
    {    
       double b1, b2, b3; //coordenadas baricentricas
       inicializarBari(double& b1, double& b2, double& b3);
       //b1 = b2 = b3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente

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
						break;
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
