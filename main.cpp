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

int id_atual = 300; // começa no triangulo 1
double pontoX = 0.0; // variavel que armazena o pontoX 
double pontoY = 0.0; // variavel que armazena o pontoY

// Declaração das funções:
/*
1- captura coordenada
2- realiza calculos baricentricos
3- Procura qual o triangulo que foi clicado
*/
void getCoordenates();
void baricentrico(double px, double py);
void CalculateBari(double px, double py, int id, double& b1, double& b2, double& b3);

//Funções
void getCoordenates(){
	double px, py;
	pontoX = Interactor->getPXD();
	pontoY = Interactor->getPYD();
	px = pontoX;
	py = pontoY;
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

void CalculateBari(double px, double py, int id, double& b1, double& b2, double& b3){
	/* A formula para calcularas coordenadas baricentricas se baseia no metodo:
	1- Obter as coordenadas dos vértices do triangulo
	2- calcular a area dos triangulos internos com base nas coordenadas dadas 
	3- Realizar a divisão entre os triangulos para gerar as coordenadas baricentricas
	*/

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

void RenderScene(void){ 
	allCommands->Execute();
	Print->Vertices(malha,blue,3);
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
