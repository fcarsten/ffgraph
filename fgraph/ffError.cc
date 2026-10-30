#include<stdlib.h>
#include<stream.h>
#include <ffgraph/ffError.h>

kindErrorHandler *userKindErrorHandler= 0;
charErrorHandler *userCharErrorHandler= 0;

void _ffDefError(errGrade grade, errKind kind, char const *file, int line)
{
 if(grade > ERRORS_WITH_FILE_AND_LINENUMBER)
  cerr << "Error: " << kind << " in " << file << ":" << line <<endl;
 else
  cerr << "Error: " << kind << endl;
}	

void _ffDefError(errGrade grade, char const *msg, char const *file, int line)
{
 if(grade > ERRORS_WITH_FILE_AND_LINENUMBER)
  cerr << "Error: " << msg << " in " << file << ":" << line <<endl;
 else
  cerr << "Error: " << msg << endl;
}	

void _ffError(errGrade grade, errKind kind, char const *file, int line)
{
  if(userKindErrorHandler != NULL)
    (*userKindErrorHandler)(grade, kind, file, line);
  else
    _ffDefError(grade, kind, file, line);
}	

void _ffError(errGrade grade, char const *msg, char const *file, int line)
{
  if(userCharErrorHandler != NULL)
    (*userCharErrorHandler)(grade, msg, file, line);
  else
    _ffDefError(grade, msg, file, line);
}	

void setErrorHandler(kindErrorHandler keh, charErrorHandler ceh)
{
  if(keh)
    userKindErrorHandler = keh;
  if(ceh)
    userCharErrorHandler = ceh;
}
