#ifndef CMD_CHOICE_FIRST_TRIANGLE_HPP
#define CMD_CHOICE_FIRST_TRIANGLE_HPP

#include <string>
#include "../../scrInteractor.h"
#include "Command.hpp"

class CmdChoiceFirstTriangle : public Command{
public:
   typedef Command      TCommand;
   
public:
   scrInteractor     *interactor_;
   
public:

    CmdChoiceFirstTriangle(scrInteractor *interactor, int& x, int& y):TCommand(numModes_i, mode_i), interactor_(Interactor){
        x = this->getPXD();
        y = this->getPYD();
        this->helpMessage_ = "chose the origin of the search";
    }

   
public:

   virtual
   void
   Execute(){
      if(this->currentMode_==1){
         this->interactor_->WriteScreenImage();
         this->currentMode_=0;
      }
   }; 

   
};

#endif /*CMDSHOWMESH_HPP_*/
