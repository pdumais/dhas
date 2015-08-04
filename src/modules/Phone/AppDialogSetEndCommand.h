#ifndef APPDIALOGSETENDCOMMAND_H
#define APPDIALOGSETENDCOMMAND_H
#include <resip/dum/DumCommand.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/Handle.hxx>

class AppDialogSetEndCommand : public resip::DumCommand
{
public:
   AppDialogSetEndCommand(resip::AppDialogSetHandle h);
   void executeCommand();
   Message* clone() const;
   std::ostream& encode(std::ostream& strm) const;
   std::ostream& encodeBrief(std::ostream& strm) const;
private:
     resip::AppDialogSetHandle mHandle;
};


#endif

