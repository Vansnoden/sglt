#ifndef LPM_LINEFORMATTER_H
#define LPM_LINEFORMATTER_H

//!
//! \file
//!
#include "Singleton.h"
#include <string>
using namespace std;
#include <vector>
using namespace std;

#include "Defs.h"
#include "NoDepend.h"

namespace lpm {

//!
//! \brief Formats some values into a line with a specific format
//!
//! Singleton class which provides convenient methods to format values.
//!

template<typename T>
class LineFormatter : public Singleton<LineFormatter<T> > 
{
  public:
    //! 
    //! \brief Format the given value into a vector (a line of type string) in the specified format 
    //!
    //! \param[in] vector 	T*, the array of type \a T* (template) whose elements will be formatted.
    //! \param[in] vectorLength ull, the number of elements in \a vector.
    //! \param[in,out] outputLine string, the output line which will receive the formatted values.
    //! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
    //!
    bool FormatVector(const T* vector, ull vectorLength, string& outputLine, char delimiter = DEFAULT_FIELDS_DELIMITER);

    //! 
    //! \brief Format the given values into a matrix (a vector of lines each of type string) in the specified format 
    //!
    //! \param[in] matrix 	T*, the two-dimensional array of type \a T* (template) whose elements will be formatted.
    //! \param[in] rows ull, the number of rows in \a matrix.
    //! \param[in] columns ull, the number of columns in \a matrix.
    //! \param[in,out] outputLines vector<string>, the output vector of lines which will receive the formatted matrix.
    //! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
    //!
    bool FormatMatrix(const T* matrix, ull rows, ull columns, vector<string>& outputLines, char delimiter = DEFAULT_FIELDS_DELIMITER);

};
//! 
//! \brief Format the given value into a vector (a line of type string) in the specified format 
//!
//! \param[in] vector 	T*, the array of type \a T* (template) whose elements will be formatted.
//! \param[in] vectorLength ull, the number of elements in \a vector.
//! \param[in,out] outputLine string, the output line which will receive the formatted values.
//! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
//!
//! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
//!
template<typename T>
bool LineFormatter<T>::FormatVector(const T* vector, ull vectorLength, string& outputLine, char delimiter) 
{
  // Bouml preserved body begin 00083991

	if(vector == NULL || vectorLength == 0)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	stringstream ss("");
	for(ull i = 0; i < vectorLength; i++)
	{
		ss << vector[i];
		if(i != vectorLength - 1) { ss << delimiter; }
	}

	outputLine = ss.str();

	return true;

  // Bouml preserved body end 00083991
}

//! 
//! \brief Format the given values into a matrix (a vector of lines each of type string) in the specified format 
//!
//! \param[in] matrix 	T*, the two-dimensional array of type \a T* (template) whose elements will be formatted.
//! \param[in] rows ull, the number of rows in \a matrix.
//! \param[in] columns ull, the number of columns in \a matrix.
//! \param[in,out] outputLines vector<string>, the output vector of lines which will receive the formatted matrix.
//! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
//!
//! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
//!
template<typename T>
bool LineFormatter<T>::FormatMatrix(const T* matrix, ull rows, ull columns, vector<string>& outputLines, char delimiter) 
{
  // Bouml preserved body begin 00083A11

	if(matrix == NULL || rows == 0 || columns == 0)
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	outputLines.clear();
	for(ull i = 0; i < rows; i++)
	{
		stringstream ss("");
		for(ull j = 0; j < columns; j++)
		{
			ss << matrix[GET_INDEX(i, j, columns)];
			if(j != columns - 1) { ss << delimiter; }
		}

		string outputLine = ss.str();
		outputLines.push_back(outputLine);
	}

	return true;

  // Bouml preserved body end 00083A11
}


} // namespace lpm
#endif
