#ifndef LPM_FILE_H
#define LPM_FILE_H

//!
//! \file
//!
#include <string>
using namespace std;

#include "Defs.h"

namespace lpm {

//!
//! \brief Represents a file
//!
//! Files, either input, or, output (i.e. read-only, or, read-write) are encapsulated by this class, which
//! additionally provides convenient methods to read/write from/to them. 
//!
//! \note After an object of type File is created, it is good practice to ensure that IsGood() returns \a true.
//! This allows to make sure that the library will able to read/write the file.
//! 
//! \see IsGood(), ReadNextLine(), WriteLine()
//!
class File 
{
  public:
    //! 
    //! \brief Constructs a File object given a \a path.
    //!
    //! \param[in] path 	string, the filepath.
    //! \param[in] readOnly 	[optional] bool, the access mode to the file (read-only, if \a readOnly = \a true, read-write, otherwise)
    //!
    //! \note If \a readOnly = \a false (i.e. the access mode is read-write), the filepath need not point to an existing file. 
    //! If it does not, the file will be created. In either case, the directory structure must exist.
    //!
    File(string path, bool readOnly = true);

    virtual ~File();


  private:
    string path;

    std::fstream stream;

    bool readOnly;

    std::fstream& GetStream() const;


  public:
    //! 
    //! \brief Returns the status of the file
    //!
    //! \return true or false, depending on whether the file has been opened successfully and can be accessed as expected
    //!
    bool IsGood() const;

    //! 
    //! \brief Returns whether the end of the file has been reached
    //!
    //! \return true or false, depending on whether the end of the file has been reached
    //!
    bool IsEOF() const;

    //! 
    //! \brief Rewinds the file so that the next read starts at the beginning
    //!
    //! \note It does not make sense to Rewind an output file.
    //!
    //! \return true or false, depending on whether the operation was successful.
    //!
    bool Rewind() const;

    string GetFilePath() const;

    //! 
    //! \brief Reads the next line in the file
    //!
    //! \param[in,out] line 	string, the output line.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    bool ReadNextLine(string& line) const;

    //!
    //! \brief Writes the given line to the end of the file
    //!
    //! \param[in] line 	string, the line to append to the file.
    //!
    //! \return true or false, depending on whether the call is successful
    //!
    bool WriteLine(const string line) const;

};

} // namespace lpm
#endif
