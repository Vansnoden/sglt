
#include "../include/ContextAnalysisSchedule.h"
#include "../include/LoadContextOperation.h"
#include "../include/ContextAnalysisOperation.h"

namespace lpm {

ContextAnalysisSchedule::ContextAnalysisSchedule(string name, ContextAnalysisOperation* analysis, LoadContextOperation* load) 
{
  // Bouml preserved body begin 000B9791

	this->name = name;
	contextAnalysisOperation = analysis;
	loadContextOperation = load;

	VERIFY(contextAnalysisOperation != NULL && loadContextOperation != NULL);

	contextAnalysisOperation->AddRef();
	loadContextOperation->AddRef();

  // Bouml preserved body end 000B9791
}

ContextAnalysisSchedule::~ContextAnalysisSchedule() 
{
  // Bouml preserved body begin 000B9811

	if(contextAnalysisOperation != NULL){ contextAnalysisOperation->Release(); }
	if(loadContextOperation != NULL){ loadContextOperation->Release(); }

  // Bouml preserved body end 000B9811
}


} // namespace lpm
