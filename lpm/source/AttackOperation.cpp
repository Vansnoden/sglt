//!
//! \file
//!
#include "../include/AttackOperation.h"
#include "../include/MetricOperation.h"
#include "../include/FilterOperation.h"
#include "../include/Context.h"

namespace lpm {

AttackOperation::AttackOperation(string name) : Operation<TraceSet, AttackOutput>(name) 
{
  // Bouml preserved body begin 00049491

	context = NULL;
	applicationPDF = NULL;
	lppmPDF = NULL;

  // Bouml preserved body end 00049491
}

AttackOperation::~AttackOperation() 
{
  // Bouml preserved body begin 00049511

	if(context != NULL) { context->Release(); }

  // Bouml preserved body end 00049511
}

//! 
//! \brief Sets the PDFs given to the attack
//!
//! \param[in] app 	FilterFunction*, an object encompassing the PDF of the application operation.
//! \param[in] lppm 	FilterFunction*, an object encompassing the PDF of the LPPM operation.
//!
//! \return true or false, depending on whether the call is successful
//!
bool AttackOperation::SetPDFs(const FilterFunction* app, const FilterFunction* lppm) 
{
  // Bouml preserved body begin 0004B091

	if(app == NULL || lppm == NULL) { return false; }

	applicationPDF = const_cast<FilterFunction*>(app);
	lppmPDF = const_cast<FilterFunction*>(lppm);

	// ugly hack to get the LPPM's flags
	LPPMOperation* lppmOp = dynamic_cast<LPPMOperation*>(lppmPDF);
	lppmFlags = lppmOp->GetFlags(); lppmOp = NULL;
	// don't abuse this!!

	return true;

  // Bouml preserved body end 0004B091
}

string AttackOperation::GetDetailString() 
{
  // Bouml preserved body begin 0004B291

	return "AttackOperation: " + operationName + "()";

  // Bouml preserved body end 0004B291
}

//! 
//! \brief Sets the context
//!
//! \param[in] newContext 	Context*, the new context
//!
//! \return nothing
//!
void AttackOperation::SetContext(const Context* newContext) 
{
  // Bouml preserved body begin 00052191

	context = const_cast<Context*>(newContext);
	context->AddRef();

  // Bouml preserved body end 00052191
}

void AttackOperation::SetLPPMFlags(LPPMFlags flags) 
{
  // Bouml preserved body begin 000CF211

	lppmFlags = flags;

  // Bouml preserved body end 000CF211
}


} // namespace lpm
