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
#include "VisOf/scrInteractor.h"


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
//EP
int trianguloBuscador = 1; //Inicia no id 1/
double coordenadaX = 0.0;
double coordenadaY = 0.0;

void iniciar();
void baricentros(double x4, double y4, int id, double& b1, double& b2, double& b3);
int verificaDentro(double b1, double b2, double b3);
void caminho(double pontoX, double pontoY, int identificador);

void iniciar(){
	double pontoX2 = Interactor->pegarX2();
	double pontoY2 = Interactor->pegarY2();
	double bari1 = 0.0;
	double bari2 = 0.0;
	double bari3 = 0.0;
	int cont = 0;

	while(cont<malha->getNumberOfCells()){
		bari1 = -1;
		bari2 = -1;
		bari3 = -1;
		//voltaraqui
		baricentros(pontoX2, pontoY2, cont, bari1, bari2, bari3);
		if (bari1>0 && bari2>0 && bari3>0){
			trianguloBuscador = cont;
			break;
		}//voltar aqui

		cont++;
	}
}

void baricentros(double x4, double y4, int id, double& b1, double& b2, double& b3){//MEtodo que calcula as coordenadas baricentricas e coloca nas vaariaveis apontadas pelos endereços

	//ALTERAR PARA ARRAY o b1 b2 b3


	//Coordenadas dos pontos 1, 2 e 3 no triangulo:
	double x1, y1, x2, y2, x3, y3;
	x1 = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(0);
	y1 = malha->getVertex(malha->getCell(id)->getVertexId(0))->getCoord(1);
	x2 = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(0);
	y2 = malha->getVertex(malha->getCell(id)->getVertexId(1))->getCoord(1);
	x3 = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(0);
	y3 = malha->getVertex(malha->getCell(id)->getVertexId(2))->getCoord(1);

	//NEcessa´rio calcular a area do triangulo. Dada as circunstancias, é melhor calcular utilizando A = x1y2 + x2y3 + x3y1 – x1y3 – x2y1 – x3y2.
	double A123 = 0.5*((x1*y2)-(y1*x2)+(y1*x3)-(x1*y3)+(x2*y3)-(y2*x3));//area do triangulo 123
	double A423 = 0.5*((x4*y2)-(y4*x2)+(y4*x3)-(x4*y3)+(x2*y3)-(y2*x3));//area do triangulo 423
	double A143 = 0.5*((x1*y4)-(y1*x4)+(y1*x3)-(x1*y3)+(x4*y3)-(y4*x3));//area do triangulo 143
	double A124 = 0.5*((x1*y2)-(y1*x2)+(y1*x4)-(x1*y4)+(x2*y4)-(y2*x4));//area do triangulo 124
	
	//Calcular as coordenadas baricentricas usando as areas calculadas acima
	b1 = A423/A123;
	b2 = A143/A123;
	b3 = A124/A123;
}

int verificaDentro(double b1, double b2, double b3){
	if(b1 > 0 && b2 > 0 && b3 > 0) return 0;//o ponto está dentro do triangulo
	else if(b1 < b2 && b1 < b3) return 1;//b1 é o menor
	else if(b2 < b1 && b2 < b3) return 2;//b2 é o menor
	else return 3;//b3 é o menor
}

void caminho(double pontoX, double pontoY, int identificador){
	double b1, b2, b3, x1, y1, x2, y2, x3, y3;
	int seguinte, menor;
	int vertice[2];
	int retorno;
	while(b1 <= 0 || b2 <= 0 || b3 <= 0){
		//Triagulo atual
		Print->Face(malha->getCell(identificador), dgreen);
		//coordenadas
		baricentros(pontoX, pontoY, identificador, b1, b2, b3);
		//verifica se está dentro
		retorno = verificaDentro(b1, b2, b3);
		if (retorno == 0) break;
		else {
			seguinte = malha->getCell(identificador)->getMateId(retorno);
			//verifica se está foda da area de trabalho
			if (seguinte == -1)
			{
			   //imprime vertice que direcionará para o triangulo
			   if (retorno == 1) //BC
			   { 
				  vertice[0] = 1;
				  vertice[1] = 2;
			   }
			   else if (menor == 2) //CA
			   {
					vertice[0] = 2;
					vertice[1] = 0;
			   }
			   else //AB
			   {
					vertice[0] = 0;
					vertice[1] = 1;
			   }
			   
			   Print->Edge(malha->getVertex(malha->getCell(identificador)->getVertexId(vertice[0])), malha->getVertex(malha->getCell(identificador)->getVertexId(vertice[1])), black, 3.0);
			   break;
			}
		}
		identificador = seguinte;
	}
}


void RenderScene(void){ 
	allCommands->Execute();
	Print->Vertices(malha,blue,3);

	//primeira face
    Print->Face(malha->getCell(trianguloBuscador), yellow);
    
    //Defini inicio
    if (Interactor->pegarClick2() == true)
    {
       iniciar();
       coordenadaX = Interactor->pegarX();
       coordenadaY = Interactor->pegarY();
       Print->FacesWireframe(malha,grey,3);
   	   glFinish();
	   glutSwapBuffers();
    }
    //Pinta a tela
    if (coordenadaX != Interactor->pegarX() || coordenadaY != Interactor->pegarY())
    {
       caminho(Interactor->pegarX(), Interactor->pegarY(), trianguloBuscador);
       Print->FacesWireframe(malha,grey,3);
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

}

using namespace std;

int main(int *argc, char **argv)
{

  ofRuppert2D<MyofDefault2D> ruppert;
  ofPoints2DReader<MyofDefault2D> reader;
  ofVtkWriter<MyofDefault2D> writer;
  Interactor->setDraw(RenderScene);
	meshHandler.Set(new TMesh());
      char *fileBrasil = "/mnt/c/Users/pdc18/Desktop/epHelton/Brasil.off";

     
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
