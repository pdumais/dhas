#ifndef APPDIALOGSETRONACOMMAND_H
#define APPDIALOGSETRONACOMMAND_H
#include <resip/dum/DumCommand.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/Handle.hxx>

class AppDialogSetRONACommand : public resip::DumCommand
{
public:
   AppDialogSetRONACommand(resip::AppDialogSetHandle h);
   void executeCommand();
   Message* clone() const;
   std::ostream& encode(std::ostream& strm) const;
   std::ostream& encodeBrief(std::ostream& strm) const;
private:
     resip::AppDialogSetHandle mHandle;
};


#endif

