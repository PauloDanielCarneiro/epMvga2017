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
void EP();

//Busca pelo inicio do problema
void getInicio(bool clique_direito)
{
     if (clique_direito)
     {
		int i = 0;
		int celulas = malha->getNumberOfCells();
	    for (i = 0; i < celulas; i++)
	    {
            double xp = Interactor->getPXD(); //coordenada x
            double yp = Interactor->getPYD(); //coordenada y
            double b1, b2, b3; //coordenadas baricentricas
            b1 = b2 = b3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente
            double xa, ya, xb, yb, xc, yc; 
			xa = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(0);
			ya = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(1);
			xb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(0);
			yb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(1);
			xc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(0);
			yc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(1);
			
			double ABC = 0.5*((xa*yb)-(ya*xb)+(ya*xc)-(xa*yc)+(xb*yc)-(yb*xc));
			double PBC = 0.5*((xp*yb)-(yp*xb)+(yp*xc)-(xp*yc)+(xb*yc)-(yb*xc));
			double APC = 0.5*((xa*yp)-(ya*xp)+(ya*xc)-(xa*yc)+(xp*yc)-(yp*xc));
			double ABP = 0.5*((xa*yb)-(ya*xb)+(ya*xp)-(xa*yp)+(xb*yp)-(yb*xp));
			
			b1 = PBC/ABC;
			b2 = APC/ABC;
			b3 = ABP/ABC;
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
        }
     }
}


//ep
void EP(){
    double xp, yp; //coordenadas do ponto P
    xp = Interactor->getPX();
    yp = Interactor->getPY();
    
    int id = id_atual;
    
    if (xp != 0.0 && yp != 0.0)
    {    
       double b1, b2, b3; //coordenadas baricentricas
       b1 = b2 = b3 = -1; //inicializar valores das coordenadas baricentricas arbitrariamente

       for (;b1 <= 0 || b2 <= 0 || b3 <= 0;)
       {    
          Print->Face(malha->getCell(id), dgreen);
          
          double xa, ya, xb, yb, xc, yc; 
			xa = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(0);
			ya = malha->getVertex(malha->getCell(i)->getVertexId(0))->getCoord(1);
			xb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(0);
			yb = malha->getVertex(malha->getCell(i)->getVertexId(1))->getCoord(1);
			xc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(0);
			yc = malha->getVertex(malha->getCell(i)->getVertexId(2))->getCoord(1);
			
			double ABC = 0.5*((xa*yb)-(ya*xb)+(ya*xc)-(xa*yc)+(xb*yc)-(yb*xc));
			double PBC = 0.5*((xp*yb)-(yp*xb)+(yp*xc)-(xp*yc)+(xb*yc)-(yb*xc));
			double APC = 0.5*((xa*yp)-(ya*xp)+(ya*xc)-(xa*yc)+(xp*yc)-(yp*xc));
			double ABP = 0.5*((xa*yb)-(ya*xb)+(ya*xp)-(xa*yp)+(xb*yp)-(yb*xp));
			
			b1 = PBC/ABC;
			b2 = APC/ABC;
			b3 = ABP/ABC;

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




//alterado para usar no ep
void RenderScene(void){
	allCommands->Execute();
	
	Print->Vertices(malha,blue,3);
    Print->Face(malha->getCell(id_atual), yellow);


    getInicio(Interactor->getMouseRight());
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
