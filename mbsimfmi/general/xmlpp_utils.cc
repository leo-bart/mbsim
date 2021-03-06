#include "config.h"
#include "xmlpp_utils.h"
#include <mbsimxml/mbsimflatxml.h>

using namespace std;
using namespace MBXMLUtils;
using namespace MBSim;

namespace MBSimFMI {

void convertXPathParamSetToVariable(const std::shared_ptr<Preprocess::XPathParamSet> &xpathParam,
                                    vector<std::shared_ptr<Variable> > &fmiParam, const std::shared_ptr<Eval> &eval) {
  // only XML parameters of the DynamicSystemSolver element are used. -> search this element (as xpath expression)
  Preprocess::XPathParamSet::iterator parSetIt=
    xpathParam->find("/{"+MBSIMXML.getNamespaceURI()+"}MBSimProject[1]/{"+MBSIM.getNamespaceURI()+"}DynamicSystemSolver[1]");
  // if it exists loop over all parameters of this (Embed) element
  if(parSetIt!=xpathParam->end()) {
    for(Preprocess::ParamSet::iterator pIt=parSetIt->second.begin(); pIt!=parSetIt->second.end(); ++pIt) {
      // add a scalar variable as it
      if(eval->valueIsOfType(pIt->second, Eval::ScalarType))
        fmiParam.push_back(std::make_shared<VariableStore<double> >(pIt->first,
          Parameter, eval->cast<double>(pIt->second)));
      // add a vector variable as seperate scalars with [idx]
      else if(eval->valueIsOfType(pIt->second, Eval::VectorType)) {
        vector<double> value=eval->cast<vector<double> >(pIt->second);
        for(int i=0; i<value.size(); ++i)
          fmiParam.push_back(std::make_shared<VariableStore<double> >(pIt->first+"["+to_string(i+1)+"]",
            Parameter, value[i]));
      }
      // add a matrix variable as seperate scalars with [rowidx,colIdx]
      else if(eval->valueIsOfType(pIt->second, Eval::MatrixType)) {
        vector<vector<double> > value=eval->cast<vector<vector<double> > >(pIt->second);
        for(int r=0; r<value.size(); ++r)
          for(int c=0; c<value[r].size(); ++c)
            fmiParam.push_back(std::make_shared<VariableStore<double> >(pIt->first+"["+to_string(r+1)+
              ","+to_string(c+1)+"]", Parameter, value[r][c]));
      }
      // add a string variable as it
      else if(eval->valueIsOfType(pIt->second, Eval::StringType)) {
        string value=eval->cast<string>(pIt->second);
        fmiParam.push_back(std::make_shared<VariableStore<string> >(pIt->first, Parameter, value));
      }
    }
  }
}

}
