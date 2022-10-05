#ifndef LPM_LINEPARSER_H
#define LPM_LINEPARSER_H

//!
//! \file
//!
#include "Singleton.h"
#include <string>
using namespace std;
#include <vector>
using namespace std;

#include "Defs.h"
#include "Private.h"

#define ANY_NUMBER_OF_FIELDS 0

namespace lpm {

//!
//! \brief Parse a line into fields
//!
//! Singleton class which provides convenient methods to parse input.
//!

template<typename T>
class LineParser : public Singleton<LineParser<T> > 
{
  public:
    //! 
    //! \brief Parse the given input line into fields in the specified format 
    //!
    //! \param[in] line 	string, the line to parse.
    //! \param[in,out] values 	vector<T>, the vector of type \a T (template) which will receive the parsed fields.
    //! \param[in] fieldsCount [optional] ull, the number of fields to parse (if this parameter is not specified, the parser will parse the maximum number of fields it finds).
    //! \param[in,out] parsingEndPos [optional] size_t*, a pointer to a size_t variable that will receive the position in the input string \a line where the parser stopped (this will be string::npos, if the parser parses the whole line).
    //! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
    //!
    bool ParseFields(const string line, vector<T>& values, ull fieldsCount = ANY_NUMBER_OF_FIELDS, size_t* parsingEndPos = NULL, char delimiter = DEFAULT_FIELDS_DELIMITER) const;

    //! 
    //! \brief Parse the given input line into two fields in the specified format 
    //!
    //! \param[in] line 	string, the line to parse.
    //! \param[in,out] value1 	T*, a variable, of type \a T (template), that will received the first value parsed.
    //! \param[in,out] value2 	T*, a variable, of type \a T (template), that will received the second value parsed.
    //! \param[in,out] parsingEndPos [optional] size_t*, a pointer to a size_t variable that will receive the position in the input string \a line where the parser stopped (this will be string::npos, if the parser parses the whole line).
    //! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
    //!
    //! \note This method is a particular case of ParseFields(), implemented for convenience.
    //!
    //! \see ParseFields()
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
    //!
    bool ParseTwoFields(const string line, T* value1, T* value2, size_t* parsingEndPos, char delimiter);

    //! 
    //! \brief Parse the given input line into a single field in the specified format 
    //!
    //! \param[in] line 	string, the line to parse.
    //! \param[in,out] value1 	T*, a variable, of type \a T (template), that will received the value parsed.
    //! \param[in,out] parsingEndPos [optional] size_t*, a pointer to a size_t variable that will receive the position in the input string \a line where the parser stopped (this will be string::npos, if the parser parses the whole line).
    //!
    //! \note This method is a particular case of ParseFields(), implemented for convenience.
    //!
    //! \see ParseFields()
    //!
    //! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
    //!
    bool ParseValue(const string line, T* value, size_t* parsingEndPos);

};
//! 
//! \brief Parse the given input line into fields in the specified format 
//!
//! \param[in] line 	string, the line to parse.
//! \param[in,out] values 	vector<T>, the vector of type \a T (template) which will receive the parsed fields.
//! \param[in] fieldsCount [optional] ull, the number of fields to parse (if this parameter is not specified, the parser will parse the maximum number of fields it finds).
//! \param[in,out] parsingEndPos [optional] size_t*, a pointer to a size_t variable that will receive the position in the input string \a line where the parser stopped (this will be string::npos, if the parser parses the whole line).
//! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
//!
//! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
//!
template<typename T>
bool LineParser<T>::ParseFields(const string line, vector<T>& values, ull fieldsCount, size_t* parsingEndPos, char delimiter) const 
{
  // Bouml preserved body begin 00079691

	if(line.empty() == true || delimiter == '\0')
	{
		SET_ERROR_CODE(ERROR_CODE_INVALID_ARGUMENTS);
		return false;
	}

	bool done = false;
	values.clear();

	size_t currentPos = 0;
	try
	{
		while(done == false)
		{
			T value = 0;

			size_t pos = line.find(delimiter, currentPos);
			if(parsingEndPos != NULL) { *parsingEndPos = pos; }

			if(pos == string::npos)
			{
				if(fieldsCount == ANY_NUMBER_OF_FIELDS || values.size() == fieldsCount - 1)
				{
					stringstream ss(line.substr(currentPos, string::npos));
					ss >> value;

					if(ss.fail() == true){ values.clear(); return false; }

					values.push_back(value);

					if(fieldsCount != ANY_NUMBER_OF_FIELDS) { VERIFY(values.size() == fieldsCount); }

					return true;
				}

				values.clear();
				return false;
			}

			stringstream ss(line.substr(currentPos, pos));
			ss >> value;

			if(ss.fail() == true) { values.clear(); return false; }

			values.push_back(value);

			currentPos = pos + 1; // field delimiter is 1 char

			if(fieldsCount != ANY_NUMBER_OF_FIELDS && values.size() == fieldsCount) { done = true; }
		}
	}
	catch(istream::failure e) { return false; }

	if(parsingEndPos != NULL)
	{
		string unparsed = line.substr(0, *parsingEndPos);

		// check for empty unparsed line
		bool empty = true;
		const char* l = unparsed.c_str();
		char c = *l;
		while(c != '\0')
		{
			if(isspace(c) == false && c != '\r' && c != '\n') { empty = false; break;}

			l++;
			c = *l;
		}

		if(empty == true) { *parsingEndPos = string::npos; }
	}

	if(fieldsCount != ANY_NUMBER_OF_FIELDS) { VERIFY(values.size() == fieldsCount); }

	return true;

  // Bouml preserved body end 00079691
}

//! 
//! \brief Parse the given input line into two fields in the specified format 
//!
//! \param[in] line 	string, the line to parse.
//! \param[in,out] value1 	T*, a variable, of type \a T (template), that will received the first value parsed.
//! \param[in,out] value2 	T*, a variable, of type \a T (template), that will received the second value parsed.
//! \param[in,out] parsingEndPos [optional] size_t*, a pointer to a size_t variable that will receive the position in the input string \a line where the parser stopped (this will be string::npos, if the parser parses the whole line).
//! \param[in] delimiter [optional] char, the fields delimiter to use (the default value is a comma: ',').
//!
//! \note This method is a particular case of ParseFields(), implemented for convenience.
//!
//! \see ParseFields()
//!
//! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
//!
template<typename T>
bool LineParser<T>::ParseTwoFields(string line, T* value1, T* value2, size_t* parsingEndPos, char delimiter) 
{
  // Bouml preserved body begin 00081E11

	if(value1 == NULL || value2 == NULL) { return false; }

	vector<T> values = vector<T>();

	bool success = ParseFields(line, values, 2, parsingEndPos, delimiter);

	if(success == false){ return false; }

	*value1 = values[0];
	*value2 = values[1];

	return true;

  // Bouml preserved body end 00081E11
}

//! 
//! \brief Parse the given input line into a single field in the specified format 
//!
//! \param[in] line 	string, the line to parse.
//! \param[in,out] value1 	T*, a variable, of type \a T (template), that will received the value parsed.
//! \param[in,out] parsingEndPos [optional] size_t*, a pointer to a size_t variable that will receive the position in the input string \a line where the parser stopped (this will be string::npos, if the parser parses the whole line).
//!
//! \note This method is a particular case of ParseFields(), implemented for convenience.
//!
//! \see ParseFields()
//!
//! \return true or false, depending on whether the call is successful (i.e. whether the line was parsed successfully)
//!
template<typename T>
bool LineParser<T>::ParseValue(const string line, T* value, size_t* parsingEndPos) 
{
  // Bouml preserved body begin 00081D91

	if(value == NULL) { return false; }

	vector<T> values = vector<T>();

	bool success = ParseFields(line, values, 1, parsingEndPos);

	if(success == false){ return false; }

	*value = values[0];

	return true;

  // Bouml preserved body end 00081D91
}


} // namespace lpm
#endif
