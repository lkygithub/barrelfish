//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include "header.h"
#include "RCCE.h"

void  adi() {

//---------------------------------------------------------------------
//---------------------------------------------------------------------

      copy_faces();
      x_solve();
      y_solve();
      z_solve();
      add();

      return;
}
