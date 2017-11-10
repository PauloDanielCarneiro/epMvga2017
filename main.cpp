#include<math.h>

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

//triangulo de busca
bool Inicio = false;
int id_atual = 255;
void getInicio(bool Inicio);
//double Distance(double dX0, double dY0, double dX1, double dY1);
void baricentrico(double& b1, double& b2, double& b3, double& xp, double& yp, int i);
void EP();

//Identificar triangulo de inicio
void getInicio(bool Inicio)
{
     if (Inicio)
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
            //Procura triangujlo clicado
            if (b1 > 0 && b2 > 0 && b3 > 0)
            {
				id_atual = i;
               	return;
            }
            //reseta fora do mapa
            else 
            {
				id_atual = 255;
			}
		i++;
        }
     }
}
/*
double Distance(double dX0, double dY0, double dX1, double dY1){
    return sqrt(fabs(dX1 - dX0)*fabs(dX1 - dX0) + fabs(dY1 - dY0)*fabs(dY1 - dY0));
}
*/

void baricentrico(double& b1, double& b2, double& b3, double& xp, double& yp, int i){
    double coord[6];
    int aux, aux2;
    int contador = 0;
    
    for(aux = 0; aux <= 2; aux++){
        for(aux2 = 0; aux2 <= 1; aux2++){
            coord[contador] = malha->getVertex(malha->getCell(i)->getVertexId(aux))->getCoord(aux2);
            contador++;
        }
    }
  
    double ABC = ((coord[0] * coord[3]) + (coord[2] * coord[5]) + (coord[4] * coord[1]) - (coord[0] * coord[5]) - (coord[4] * coord[3]) - (coord[2] * coord[1])) * 0.5;
    double ABP = ((coord[0] * coord[3]) + (coord[2] * yp) + (xp * coord[1]) - (coord[0] * yp) - (xp * coord[3]) - (coord[2] * coord[1])) * 0.5;
    double APC = ((coord[0] * yp) + (xp * coord[5]) + (coord[4] * coord[1]) - (coord[0] * coord[5]) - (coord[4] * yp) - (xp * coord[1])) * 0.5;
    double PBC = ((xp * coord[3]) + (coord[2] * coord[5]) + (coord[4] * yp) - (xp * coord[5]) - (coord[4] * coord[3]) - (coord[2] * yp)) * 0.5;
    
	//Coordenadas baricentricas
	b1 = PBC/ABC;
	b2 = APC/ABC;
	b3 = ABP/ABC;
}

//Resolução//
void EP(){
    double xp, yp; //coordenadas do ponto P
    xp = Interactor->getPX();
    yp = Interactor->getPY();
    
    //Primeiro triangulo
    int id = id_atual;
    // n~~ao imprimir se não tiver nenhuma alteração
    if (xp != 0.0 && yp != 0.0)
    {    
       double b1, b2, b3; //coordenadas baricentricas
       b1 = b2 = b3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente

       while (b1 <= 0 || b2 <= 0 || b3 <= 0)
       {
          //imprimir caminho
          Print->Face(malha->getCell(id), dgreen);
          
          baricentrico(b1, b2, b3, xp, yp, id);


          //Verificar se chegou no triangulo final
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




//Exibe a tela
void RenderScene(void){
	allCommands->Execute();
	
	Print->Vertices(malha,blue,3);
	//[EP] Mostra o inicio
    Print->Face(malha->getCell(id_atual), red);
    
    //Localiza o triangulo inicial
    getInicio(Inicio);
    //Realiza o exercicio proposto
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
        case 'z':
            Interactor->getScreenPointOrigem();
            Inicio = true;
        break;
        case 'x':
            Interactor->getScreenPointFinal();
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
