/*
 * moFileReader - A simple .mo-File-Reader
 * Copyright (C) 2009 Domenico Gentner (scorcher24@gmail.com)
 * Copyright (C) 2018-2021 Edgar (Edgar@AnotherFoxGuy.com)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. The names of its contributors may not be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __MOFILEREADER_SINGLE_INCLUDE_H_INCLUDED__
#define __MOFILEREADER_SINGLE_INCLUDE_H_INCLUDED__

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif /* _MSC_VER */

#include <cstring>  // this is for memset when compiling with gcc.
#include <deque>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

//-------------------------------------------------------------
// Path-Seperators are different on other OS.
//-------------------------------------------------------------
#ifndef MO_PATHSEP
#ifdef WIN32
#define MO_PATHSEP std::string("\\")
#else
#define MO_PATHSEP std::string("/")
#endif
#endif

//-------------------------------------------------------------
// Defines the beginning of the namespace moFileLib.
//-------------------------------------------------------------
#ifndef MO_BEGIN_NAMESPACE
#define MO_BEGIN_NAMESPACE namespace moFileLib {
#endif
//-------------------------------------------------------------
// Ends the current namespace.
//-------------------------------------------------------------
#ifndef MO_END_NAMESPACE
#define MO_END_NAMESPACE }
#endif

/** \mainpage moFileReaderSDK
 *
 *
 * <h2>Include in project</h2>
 *
 * Usage of this library is quite easy, simply add  moFileReader.hpp to your project. Thats all you
 * have to do. You can safely exclude mo.cpp, since this file keeps the entry-points of the .exe
 * only.
 *
 * <h2>Usage</h2>
 *
 * This is moFileReader, a simple gettext-replacement. The usage of this library is, hopefully,
 * fairly simple: \code
 *
 * // Instanciate the class
 * moFileLib::moFileReader reader;
 *
 * // Load a .mo-File.
 * if ( reader.ReadFile("myTranslationFile.mo") != moFileLib::moFileReader::EC_SUCCESS )
 * {
 *    // Error Handling
 * }
 *
 * // Now, you can lookup the strings you stored in the .mo-File:
 * std::cout << reader.Lookup("MyTranslationString") << std::endl;
 *
 * \endcode
 * Thats all! This small code has no dependencies, except the C/C++-runtime of your compiler,
 * so it should work on all machines where a C++-runtime is provided.
 *
 * \note We do not yet support .mo-Files with reversed magic-numbers, since I don't have
 *       a file to test it and I hate to release stuff I wasn't able to test.
 *
 * <h2>Changelog</h2>
 *
 * - Version 1.2.0
 *     - Proper implementation of contexts strings
 *          Now it uses a separate 2D map for storing strings with context
 *     - Fixed MagicNumber check not working on Linux
 *     - Removed duplicate code
 *     - Added option to disable convenience Class
 *
 * - Version 1.1.0
 *     - Converted library to a header-only library
 *
 * - Version 1.0.0
 *     - Added new function: LookupWithContext
 *     - Added unit-tests
 *     - Added support for packaging with Conan
 *     - Moved project to https://github.com/AnotherFoxGuy/MofileReader
 *
 * - Version 0.1.2
 *     - Generic improvements to the documentation.
 *     - Generic improvements to the code
 *     - Fixed a bug in mo.cpp which caused the application not to print the help
 *       message if only --export or --lookup where missing.
 *     - Added -h, --help and -? to moReader[.exe]. It will print the help-screen.
 *     - Added --version and -v to moReader[.exe]. It will print some informations about the
 * program.
 *     - Added --license to moReader[.exe]. This will print its license.
 *     - --export gives now a feedback about success or failure.
 *     - The HTML-Dump-Method outputs now the whole table from the empty msgid in a nice html-table,
 * not only a few hardcoded.
 *     - I had an issue-report that the Error-Constants can collide with foreign code under certain
 * conditions, so I added a patch which renamed the error-constants to more compatible names.
 *
 * - Version 0.1.1
 *     - Added the ability to export mo's as HTML.
 *     - Fixed a bug causing a crash when passing an invalid value to moFileReader::Lookup().
 *     - Added a new file, moFileConfig.h, holding the macros for the project.
 *     - Added the ability to be configured by cmake.
 *     - Added some more inline-functions, which really enhance the singleton.
 *
 * - Version 0.1.0
 *     - Initial Version and release to http://googlecode.com
 *
 *
 * <h2>Credits</h2>
 *
 * Gettext is part of the GNU-Tools and (C) by the <a href="http://fsf.org">Free Software
 * Foundation</a>.\n Visual C++ Express is a registered Trademark of Microsoft, One Microsoft Way,
 * Redmond, USA.\n All other Trademarks are property of their respective owners. \n \n Thanks for
 * using this piece of OpenSource-Software.\n Submit patches and/or bugs on
 * https://github.com/AnotherFoxGuy/MofileReader. Send your flames, dumb comments etc to /dev/null,
 * thank you.
 */

/** \namespace moFileLib
 * \brief This is the only namespace of this small sourcecode.
 */
MO_BEGIN_NAMESPACE

const std::string g_css = R"(
body {
     background-color: black;
     color: silver;
}
table {
     width: 80%;
}
th {
     background-color: orange;
     color: black;
}
hr {
     color: red;
    width: 80%;
     size: 5px;
}
a:link{
    color: gold;
}
a:visited{
    color: grey;
}
a:hover{
    color:blue;
}
.copyleft{
     font-size: 12px;
     text-align: center;
})";

/**
 * \brief Keeps the Description of translated and original strings.
 *
 *
 * To load a String from the file, we need its offset and its length.
 * This struct helps us grouping this information.
 */
struct moTranslationPairInformation {
    /// \brief Constructor
    moTranslationPairInformation() : m_orLength(0), m_orOffset(0), m_trLength(0), m_trOffset(0) {}

    /// \brief Length of the Original String
    int m_orLength;

    /// \brief Offset of the Original String (absolute)
    int m_orOffset;

    /// \brief Length of the Translated String
    int m_trLength;

    /// \brief Offset of the Translated String (absolute)
    int m_trOffset;
};

/**
 * \brief Describes the "Header" of a .mo-File.
 *
 *
 * The File info keeps the header of a .mo-file and
 * a list of the string-descriptions.
 * The typedef is for the type of the string-list.
 * The constructor ensures, that all members get a nice
 * initial value.
 */
struct moFileInfo {
    /// \brief Type for the list of all Translation-Pair-Descriptions.
    typedef std::deque<moTranslationPairInformation> moTranslationPairList;

    /// \brief Constructor
    moFileInfo()
    : m_magicNumber(0),
      m_fileVersion(0),
      m_numStrings(0),
      m_offsetOriginal(0),
      m_offsetTranslation(0),
      m_sizeHashtable(0),
      m_offsetHashtable(0),
      m_reversed(false) {}

    /// \brief The Magic Number, compare it to g_MagicNumber.
    int m_magicNumber;

    /// \brief The File Version, 0 atm according to the manpage.
    int m_fileVersion;

    /// \brief Number of Strings in the .mo-file.
    int m_numStrings;

    /// \brief Offset of the Table of the Original Strings
    int m_offsetOriginal;

    /// \brief Offset of the Table of the Translated Strings
    int m_offsetTranslation;

    /// \brief Size of 1 Entry in the Hashtable.
    int m_sizeHashtable;

    /// \brief The Offset of the Hashtable.
    int m_offsetHashtable;

    /** \brief Tells you if the bytes are reversed
     * \note When this is true, the bytes are reversed and the Magic number is like g_MagicReversed
     */
    bool m_reversed;

    /// \brief A list containing offset and length of the strings in the file.
    moTranslationPairList m_translationPairInformation;
};

/**
 * \brief This class is a gettext-replacement.
 *
 *
 * The usage is quite simple:\n
 * Tell the class which .mo-file it shall load via
 * moFileReader::ReadFile(). The method will attempt to load
 * the file, all translations will be stored in memory.
 * Afterwards you can lookup the strings with moFileReader::Lookup() just
 * like you would do with gettext.
 * Additionally, you can call moFileReader::ReadFile() for as much files as you
 * like. But please be aware, that if there are duplicated keys (original strings),
 * that they will replace each other in the lookup-table. There is no check done, if a
 * key already exists.
 *
 * \note If you add "Lookup" to the keywords of the gettext-parser (like poEdit),
 * it will recognize the Strings loaded with an instance of this class.
 * \note I strongly recommend poEdit from Vaclav Slavik for editing .po-Files,
 *       get it at http://poedit.net for various systems :).
 */
class moFileReader {
protected:
    /// \brief Type for the map which holds the translation-pairs later.
    typedef std::map<std::string, std::string> moLookupList;

    /// \brief Type for the 2D map which holds the translation-pairs later.
    typedef std::map<std::string, moLookupList> moContextLookupList;

public:
    /// \brief The Magic Number describes the endianess of bytes on the system.
    static const unsigned int MagicNumber = 0x950412DE;

    /// \brief If the Magic Number is Reversed, we need to swap the bytes.
    static const unsigned int MagicReversed = 0xDE120495;

    /// \brief The character that is used to separate context strings
    static const char ContextSeparator = '\x04';

    /// \brief The possible errorcodes for methods of this class
    enum eErrorCode {
        /// \brief Indicated success
        EC_SUCCESS = 0,

        /// \brief Indicates an error
        EC_ERROR,

        /// \brief The given File was not found.
        EC_FILENOTFOUND,

        /// \brief The file is invalid.
        EC_FILEINVALID,

        /// \brief Empty Lookup-Table (returned by ExportAsHTML())
        EC_TABLEEMPTY,

        /// \brief The magic number did not match
        EC_MAGICNUMBER_NOMATCH,

        /**
         * \brief The magic number is reversed.
         * \note This is an error until the class supports it.
         */
        EC_MAGICNUMBER_REVERSED,
    };

    /** \brief Reads a .mo-file
     * \param[in] data The path to the file to load.
     * \return SUCCESS on success or one of the other error-codes in eErrorCode on error.
     *
     * This is the core-feature. This method loads the .mo-file and stores
     * all translation-pairs in a map. You can access this map via the method
     * moFileReader::Lookup().
     */
    moFileReader::eErrorCode ParseData(const std::string &data) {
        // Opening the file.
        std::stringstream stream(data);

        return ReadStream(stream);
    }

    /** \brief Reads a .mo-file
     * \param[in] filename The path to the file to load.
     * \return SUCCESS on success or one of the other error-codes in eErrorCode on error.
     *
     * This is the core-feature. This method loads the .mo-file and stores
     * all translation-pairs in a map. You can access this map via the method
     * moFileReader::Lookup().
     */
    eErrorCode ReadFile(const char *filename) {
        // Opening the file.
        std::ifstream stream(filename, std::ios_base::binary | std::ios_base::in);
        if (!stream.is_open()) {
            m_error = std::string("Cannot open File ") + std::string(filename);
            return moFileReader::EC_FILENOTFOUND;
        }

        eErrorCode res = ReadStream(stream);
        stream.close();

        return res;
    }

    /** \brief Reads data from a stream
     * \param[in] stream
     * \return SUCCESS on success or one of the other error-codes in eErrorCode on error.
     *
     */
    template <typename T>
    eErrorCode ReadStream(T &stream) {
        // Creating a file-description.
        moFileInfo moInfo;

        // Reference to the List inside moInfo.
        moFileInfo::moTranslationPairList &TransPairInfo = moInfo.m_translationPairInformation;

        // Read in all the 4 bytes of fire-magic, offsets and stuff...
        stream.read((char *)&moInfo.m_magicNumber, 4);
        stream.read((char *)&moInfo.m_fileVersion, 4);
        stream.read((char *)&moInfo.m_numStrings, 4);
        stream.read((char *)&moInfo.m_offsetOriginal, 4);
        stream.read((char *)&moInfo.m_offsetTranslation, 4);
        stream.read((char *)&moInfo.m_sizeHashtable, 4);
        stream.read((char *)&moInfo.m_offsetHashtable, 4);

        if (stream.bad()) {
            m_error =
              "Stream bad during reading. The .mo-file seems to be invalid or has bad "
              "descriptions!";
            return moFileReader::EC_FILEINVALID;
        }

        // Checking the Magic Number
        if (MagicNumber != moInfo.m_magicNumber) {
            if (MagicReversed != moInfo.m_magicNumber) {
                m_error = "The Magic Number does not match in all cases!";
                return moFileReader::EC_MAGICNUMBER_NOMATCH;
            } else {
                moInfo.m_reversed = true;
                m_error = "Magic Number is reversed. We do not support this yet!";
                return moFileReader::EC_MAGICNUMBER_REVERSED;
            }
        }

        // Now we search all Length & Offsets of the original strings
        for (int i = 0; i < moInfo.m_numStrings; i++) {
            moTranslationPairInformation _str;
            stream.read((char *)&_str.m_orLength, 4);
            stream.read((char *)&_str.m_orOffset, 4);
            if (stream.bad()) {
                m_error =
                  "Stream bad during reading. The .mo-file seems to be invalid or has bad "
                  "descriptions!";
                return moFileReader::EC_FILEINVALID;
            }

            TransPairInfo.push_back(_str);
        }

        // Get all Lengths & Offsets of the translated strings
        // Be aware: The Descriptors already exist in our list, so we just mod. refs from the deque.
        for (int i = 0; i < moInfo.m_numStrings; i++) {
            moTranslationPairInformation &_str = TransPairInfo[i];
            stream.read((char *)&_str.m_trLength, 4);
            stream.read((char *)&_str.m_trOffset, 4);
            if (stream.bad()) {
                m_error =
                  "Stream bad during reading. The .mo-file seems to be invalid or has bad "
                  "descriptions!";
                return moFileReader::EC_FILEINVALID;
            }
        }

        // Normally you would read the hash-table here, but we don't use it. :)

        // Now to the interesting part, we read the strings-pairs now
        for (int i = 0; i < moInfo.m_numStrings; i++) {
            // We need a length of +1 to catch the trailing \0.
            int orLength = TransPairInfo[i].m_orLength + 1;
            int trLength = TransPairInfo[i].m_trLength + 1;

            int orOffset = TransPairInfo[i].m_orOffset;
            int trOffset = TransPairInfo[i].m_trOffset;

            // Original
            char *original = new char[orLength];
            memset(original, 0, sizeof(char) * orLength);

            stream.seekg(orOffset);
            stream.read(original, orLength);

            if (stream.bad()) {
                m_error =
                  "Stream bad during reading. The .mo-file seems to be invalid or has bad "
                  "descriptions!";
                return moFileReader::EC_FILEINVALID;
            }

            // Translation
            char *translation = new char[trLength];
            memset(translation, 0, sizeof(char) * trLength);

            stream.seekg(trOffset);
            stream.read(translation, trLength);

            if (stream.bad()) {
                m_error =
                  "Stream bad during reading. The .mo-file seems to be invalid or has bad "
                  "descriptions!";
                return moFileReader::EC_FILEINVALID;
            }

            std::string original_str = original;
            std::string translation_str = translation;
            auto ctxSeparator = original_str.find(ContextSeparator);

            // Store it in the map.
            if (ctxSeparator == std::string::npos) {
                m_lookup[original_str] = translation_str;
                numStrings++;
            } else {
                // try-catch for handling out_of_range exceptions
                try {
                    m_lookup_context[original_str.substr(0, ctxSeparator)][original_str.substr(
                      ctxSeparator + 1, original_str.length())] = translation_str;
                    numStrings++;
                } catch (...) {
                    m_error =
                      "Stream bad during reading. The .mo-file seems to be invalid or has bad "
                      "descriptions!";
                    return moFileReader::EC_ERROR;
                }
            }

            // Cleanup...
            delete[] original;
            delete[] translation;
        }

        // Done :)
        return moFileReader::EC_SUCCESS;
    }

    /** \brief Returns the searched translation or returns the input.
     * \param[in] id The id of the translation to search for.
     * \return The value you passed in via _id or the translated string.
     */
    std::string Lookup(const char *id) const {
        if (m_lookup.empty())
            return id;
        auto iterator = m_lookup.find(id);

        return iterator == m_lookup.end() ? id : iterator->second;
    }

    /** \brief Returns the searched translation or returns the input, restricted to the context
     * given by context. See https://www.gnu.org/software/gettext/manual/html_node/Contexts.html for
     * more info. \param[in] context Restrict to the context given. \param[in] id The id of the
     * translation to search for. \return The value you passed in via _id or the translated string.
     */
    std::string LookupWithContext(const char *context, const char *id) const {
        if (m_lookup_context.empty())
            return id;
        auto iterator = m_lookup_context.find(context);

        if (iterator == m_lookup_context.end())
            return id;
        auto iterator2 = iterator->second.find(id);

        return iterator2 == iterator->second.end() ? id : iterator2->second;
    }

    /// \brief Returns the Error Description.
    const std::string &GetErrorDescription() const {
        return m_error;
    }

    /// \brief Empties the Lookup-Table.
    void ClearTable() {
        m_lookup.clear();
        m_lookup_context.clear();
        numStrings = 0;
    }

    /** \brief Returns the Number of Entries in our Lookup-Table.
     * \note The mo-File-table always contains an empty msgid, which contains informations
     *       about the tranlsation-project. So the real number of strings is always minus 1.
     */
    unsigned int GetNumStrings() const {
        return numStrings;
    }

    /** \brief Exports the whole content of the .mo-File as .html
     * \param[in] infile The .mo-File to export.
     * \param[in] filename Where to store the .html-file. If empty, the path and filename of the
     * _infile with .html appended. \param[in,out] css The css-script for the visual style of the
     *                     file, in case you don't like mine ;).
     * \see g_css for the possible and used css-values.
     */
    static eErrorCode ExportAsHTML(const std::string &infile, const std::string &filename = "",
                                   const std::string &css = g_css) {
        // Read the file
        moFileReader reader;
        moFileReader::eErrorCode r = reader.ReadFile(infile.c_str());
        if (r != moFileReader::EC_SUCCESS) {
            return r;
        }
        if (reader.m_lookup.empty()) {
            return moFileReader::EC_TABLEEMPTY;
        }

        // Beautify Output
        std::string fname;
        size_t pos = infile.find_last_of(MO_PATHSEP);
        if (pos != std::string::npos) {
            fname = infile.substr(pos + 1, infile.length());
        } else {
            fname = infile;
        }

        // if there is no filename given, we set it to the .mo + html, e.g. test.mo.html
        std::string htmlfile(filename);
        if (htmlfile.empty()) {
            htmlfile = infile + std::string(".html");
        }

        // Ok, now prepare output.
        std::ofstream stream(htmlfile.c_str());
        if (stream.is_open()) {
            stream
              << R"(<!DOCTYPE HTML PUBLIC "- //W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">)"
              << std::endl;
            stream << "<html><head><style type=\"text/css\">\n" << std::endl;
            stream << css << std::endl;
            stream << "</style>" << std::endl;
            stream << R"(<meta http-equiv="content-type" content="text/html; charset=utf-8">)"
                   << std::endl;
            stream << "<title>Dump of " << fname << "</title></head>" << std::endl;
            stream << "<body>" << std::endl;
            stream << "<center>" << std::endl;
            stream << "<h1>" << fname << "</h1>" << std::endl;
            stream << R"(<table border="1"><th colspan="2">Project Info</th>)" << std::endl;

            std::stringstream parsee;
            parsee << reader.Lookup("");

            while (!parsee.eof()) {
                char buffer[1024];
                parsee.getline(buffer, 1024);
                std::string name;
                std::string value;

                reader.GetPoEditorString(buffer, name, value);
                if (!(name.empty() || value.empty())) {
                    stream << "<tr><td>" << name << "</td><td>" << value << "</td></tr>"
                           << std::endl;
                }
            }
            stream << "</table>" << std::endl;
            stream << "<hr noshade/>" << std::endl;

            // Now output the content
            stream << R"(<table border="1"><th colspan="2">Content</th>)" << std::endl;
            for (const auto &it : reader.m_lookup) {
                if (!it.first.empty())  // Skip the empty msgid, its the table we handled above.
                {
                    stream << "<tr><td>" << it.first << "</td><td>" << it.second << "</td></tr>"
                           << std::endl;
                }
            }
            stream << "</table><br/>" << std::endl;

            // Separate tables for each context
            for (const auto &it : reader.m_lookup_context) {
                stream << R"(<table border="1"><th colspan="2">)" << it.first << "</th>"
                       << std::endl;
                for (const auto &its : it.second) {
                    stream << "<tr><td>" << its.first << "</td><td>" << its.second << "</td></tr>"
                           << std::endl;
                }
                stream << "</table><br/>" << std::endl;
            }

            stream << "</center>" << std::endl;
            stream << "<div class=\"copyleft\">File generated by <a "
                      "href=\"https://github.com/AnotherFoxGuy/MofileReader\" "
                      "target=\"_blank\">moFileReaderSDK</a></div>"
                   << std::endl;
            stream << "</body></html>" << std::endl;
            stream.close();
        } else {
            return moFileReader::EC_FILENOTFOUND;
        }

        return moFileReader::EC_SUCCESS;
    }

protected:
    /// \brief Keeps the last error as String.
    std::string m_error;

    /** \brief Swap the endianness of a 4 byte WORD.
     * \param[in] in The value to swap.
     * \return The swapped value.
     */
    unsigned long SwapBytes(unsigned long in) {
        unsigned long b0 = (in >> 0) & 0xff;
        unsigned long b1 = (in >> 8) & 0xff;
        unsigned long b2 = (in >> 16) & 0xff;
        unsigned long b3 = (in >> 24) & 0xff;

        return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
    }

private:
    // Holds the lookup-table
    moLookupList m_lookup;
    moContextLookupList m_lookup_context;

    int numStrings = 0;

    // Replaces < with ( to satisfy html-rules.
    static void MakeHtmlConform(std::string &_inout) {
        std::string temp = _inout;
        for (unsigned int i = 0; i < temp.length(); i++) {
            if (temp[i] == '>') {
                _inout.replace(i, 1, ")");
            }
            if (temp[i] == '<') {
                _inout.replace(i, 1, "(");
            }
        }
    }

    // Extracts a value-pair from the po-edit-information
    bool GetPoEditorString(const char *_buffer, std::string &_name, std::string &_value) {
        std::string line(_buffer);
        size_t first = line.find_first_of(':');

        if (first != std::string::npos) {
            _name = line.substr(0, first);
            _value = line.substr(first + 1, line.length());

            // Replace <> with () for Html-Conformity.
            MakeHtmlConform(_value);
            MakeHtmlConform(_name);

            // Remove spaces from front and end.
            Trim(_value);
            Trim(_name);

            return true;
        }
        return false;
    }

    // Removes spaces from front and end.
    static void Trim(std::string &_in) {
        if (_in.empty()) {
            return;
        }

        _in.erase(0, _in.find_first_not_of(" "));
        _in.erase(_in.find_last_not_of(" ") + 1);

        /*while (_in[0] == ' ')
        {
            _in = _in.substr(1, _in.length());
        }
        while (_in[_in.length()] == ' ')
        {
            _in = _in.substr(0, _in.length() - 1);
        }*/
    }
};

#ifndef MO_NO_CONVENIENCE_CLASS
/** \brief Convenience Class
 *
 *
 * This class derives from moFileReader and builds a singleton to access its methods
 * in a global manner.
 * \note This class is a Singleton. Please access it via moFileReaderSingleton::GetInstance()
 * or use the provided wrappers:\n
 * - moReadMoFile()
 * - _()
 * - moFileClearTable()
 * - moFileGetErrorDescription()
 * - moFileGetNumStrings();
 */
class moFileReaderSingleton : public moFileReader {
private:
    // Private Contructor and Copy-Constructor to avoid
    // that this class is instanced.
    moFileReaderSingleton() {}

    moFileReaderSingleton(const moFileReaderSingleton &);

    moFileReaderSingleton &operator=(const moFileReaderSingleton &) {
        return *this;
    }

public:
    /** \brief Singleton-Accessor.
     * \return A static instance of moFileReaderSingleton.
     */
    static moFileReaderSingleton &GetInstance() {
        static moFileReaderSingleton theoneandonly;
        return theoneandonly;
    }
};

/** \brief Reads the .mo-File.
 * \param[in] _filename The path to the file to use.
 * \see moFileReader::ReadFile() for details.
 */
inline moFileReader::eErrorCode moReadMoFile(const char *_filename) {
    moFileReader::eErrorCode r = moFileReaderSingleton::GetInstance().ReadFile(_filename);
    return r;
}

/** \brief Looks for the spec. string to translate.
 * \param[in] id The string-id to search.
 * \return The translation if found, otherwise it returns id.
 */
inline std::string _(const char *id) {
    std::string r = moFileReaderSingleton::GetInstance().Lookup(id);
    return r;
}

/// \brief Resets the Lookup-Table.
inline void moFileClearTable() {
    moFileReaderSingleton::GetInstance().ClearTable();
}

/// \brief Returns the last known error as string or an empty class.
inline std::string moFileGetErrorDescription() {
    std::string r = moFileReaderSingleton::GetInstance().GetErrorDescription();
    return r;
}

/// \brief Returns the number of entries loaded from the .mo-File.
inline int moFileGetNumStrings() {
    int r = moFileReaderSingleton::GetInstance().GetNumStrings();
    return r;
}
#endif

MO_END_NAMESPACE

#if defined(_MSC_VER)
#pragma warning(default : 4251)
#endif /* _MSC_VER */

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif /* __MOFILEREADER_SINGLE_INCLUDE_H_INCLUDED__ */