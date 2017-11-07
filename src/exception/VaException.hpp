/** @mainpage VaException Documentation
    @brief    A little more convinient tool for throwing exceptions.

              $Version: 17.01.09 $
              $Copyright: (C) Vladislav Aleinik (valeinik00@gmail.com) $
              $Date: 2017-01-09 22:13:37 +0400$

    @par      What is VaException?
              VaException (Vladik Aleinik Exception) is just like std::exception, but it gives more information on the error
              (it allows you to pass line, function and file, in which the exception was thrown).
	          To learn more watch VaExc::Exception docs.

	@warning  VaException is currently (and forever) in alpha state, so bugs can occur.
*/

#ifndef HEADER_GUARD_VA_EXCEPTION_HPP_INCLUDED
#define HEADER_GUARD_VA_EXCEPTION_HPP_INCLUDED "VaException.hpp"

// Includes:
#include <exception>
#include <cstdio>
#include <cstring>
#include <utility>
#include <cstdlib>

/** @brief  Namespace with the whole code.

            This is for convinience and recognition, just like namespace std.

    @code
            VaExc::Exception(...);
    @endcode
*/
namespace VaExc
{
	/** @brief  Control over program behavior.

	            Contains variables, which tell the program how much information (in bytes) an Exception can store.
	*/
	namespace _control
	{
		// Some bounds:
		const size_t MAX_INFO_SIZE = 400; ///< The maximumum amount of bytes an Exception can store.
		const size_t MAX_MSG_SIZE  = 200; ///< The maximumum amount of bytes an explanation message in Exception can store.
		const size_t MAX_EXC_COUNT = 4;   ///< The maximumum amount of Exception instances in "caused-by" chains.

		const size_t NOT_ENOUGH_SPACE_SIZE = 4; ///< The size of NOT_ENOUGH_SPACE array.
		const char   NOT_ENOUGH_SPACE[NOT_ENOUGH_SPACE_SIZE + 1] = "\n..."; ///< The string, which is printed out in case there's not enough space to print something.

	} // namespace _control

	/// Wrappers to be used in Exception constructor (Exception()), mostly not for user use.
	namespace _wrappers
	{
		/** @brief  Wrapper that stores exception message.

		    @see    Exception
		    @code
		            throw Exception(..., ArgMsg("Too many kitten explosions per second. %ull total", kitExpPerSec), ...);
		    @endcode
		*/
		struct ArgMsg
		{
			char msg[_control::MAX_MSG_SIZE + 1];

			/** @brief   ArgMsg Constructor

				@see     ArgMsg
			*/
			template <typename... Args>
			explicit ArgMsg(const char* format, Args&&... args) noexcept :
				msg ()
			{
				int written = std::snprintf(msg, sizeof(msg), format, std::forward<Args>(args)...);

				if (written < 0)
				{
					written = std::sprintf(msg, "%s", _control::NOT_ENOUGH_SPACE);

					if (written < 0)
					{
						msg[0] = '\0';
					}
				}
			}
		};

		/** @brief  Just like ArgMsg, but for c-style strings (not for user).

		    @see    Exception, operator""_msg()
		*/
		struct ArgMsgConstexpr
		{
			const char* msg;
		};

		/** @brief  Wrapper to store filename of the file in which Exception is created.

		    @see    Exception, operator""_file()
		*/
		struct ArgFilename
		{
			const char* file;

			constexpr ArgFilename(const char* filename) :
				file (filename)
			{}
		};

		/** @brief  Wrapper to store function name in which Exception is created.

		    @see    Exception, operator""_func()
		*/
		struct ArgFunction
		{
			const char* func;

			constexpr ArgFunction(const char* funcName) :
				func (funcName)
			{}
		};

		/** @brief  Wrapper to store call line number in which Exception is created.

		    @see    Exception, operator""_line()
		*/
		struct ArgLine
		{
			size_t line;

			constexpr ArgLine(size_t lineNum) :
				line (lineNum)
			{}
		};

	}; // namespace _wrappers

	/// User-defined literals for convinient wrapping.
	namespace _literals
	{
		/** @brief  Exception message.

		    @see    Exception, VAEXC_POS
		    @par    Examples
		    @usage @code
		            Exception("Oh no! Exception happened!"_msg);
		    @endcode
		*/
		constexpr _wrappers::ArgMsgConstexpr operator""_msg(const char* str, size_t) noexcept { return {str}; }

		/** @brief  Filename of the file in which Exception is created.

		    @see    Exception, VAEXC_POS
		    @par    Examples
		    @usage @code
		            Exception("src/File.hpp"_file);

		            Exception(VAEXC_POS); // sometimes better option

		    @endcode
		*/
		constexpr _wrappers::ArgFilename operator""_file(const char* str, size_t) noexcept { return {str}; }

		/** @brief  Function name in which Exception is created.

		    @see    Exception, VAEXC_POS
		    @par    Examples
		    @usage @code
		            void foo()
		            {
		            	throw Exception("void foo()"_func);

		            	throw Exception(VAEXC_POS);  // a little better option
		            }
		    @endcode
		*/
		constexpr _wrappers::ArgFunction operator""_func(const char* str, size_t) noexcept { return {str}; }

		/** @brief  Call line number in which Exception is created.

		    @see    Exception, VAEXC_POS
		    @par    Examples
		    @usage @code
		            throw Exception(228_line);

		            throw Exception(VAEXC_POS); // ALWAYS MUCH BETTER
		    @endcode
		*/
		constexpr _wrappers::ArgLine operator""_line(unsigned long long int line) noexcept
		{
			// Code can't have more then that(vvvv) lines.
			// Human-made code can't.
			return {static_cast<size_t>(line)};
		}

	} // namespace _literals

	/// Namespace for ErrorInfo struct to store information conviniently (not for user).
	namespace _errorInfo
	{
		/// Class, which is only useful in Exception implementation (not for user).
		class ErrorInfo
		{
		public:
				ErrorInfo() noexcept :
					info_   (),
					filled_ (0)
				{
					std::memset(info_, 0, sizeof(info_));
				}

				ErrorInfo(const ErrorInfo& that) = default;

				ErrorInfo& operator=(const ErrorInfo& that) = default;

				~ErrorInfo() = default;

			// Interface:
				bool writeInfoWithCaption(const char* caption, const char* text) noexcept
				{
					size_t captionLen = std::strlen(caption);
					size_t    textLen = std::strlen(   text);

					if (filled_ + captionLen + textLen + _control::NOT_ENOUGH_SPACE_SIZE > _control::MAX_INFO_SIZE)
					{
						std::strcpy(info_ + filled_, _control::NOT_ENOUGH_SPACE);

						filled_ += _control::NOT_ENOUGH_SPACE_SIZE;

						return false;
					}
					else
					{
						std::strcpy(info_ + filled_, caption);
						filled_ += captionLen;

						std::strcpy(info_ + filled_, text);
						filled_ += textLen;

						return true;
					}
				}

				const char* info() const noexcept
				{
					return reinterpret_cast<const char*>(info_);
				}

		private:
			// Variables:
				char info_[_control::MAX_INFO_SIZE + 1];
				size_t filled_;

		};

	} //  namespace _errorInfo

	/** @brief  The Exception

	            Exception is just like std::exception, but it gives more information on the error (func, line, file),
	            Exception::what() can print arguments in any order ([msg, line, func] or [func, msg, line]),
	            Exception is inherited from std::exception (thus, it is a std::exception),
	            Exception supports formatted strings, just like std::printf.

	    @see    operator""_msg(),
	            operator""_file(),
	            operator""_func(),
	            operator""_line(),
	            _wrappers::ArgMsg::ArgMsg(),
	            VAEXC_POS

	    @par    Examples
	    @usage @code
	            try
	            {
	            	using namespace VaExc;

                	// ArgMsg is implemented via std::sprintf, so formatting is the same.
	            	throw Exception(ArgMsg("Error code: %d", error_code), VAEXC_POS);

	            	...
                }
	            catch (Exception& exc)
	            {
	            	throw Exception("Exception occured!!!", exc);
	            }

                ...

	            try
	            {
	            	using namespace VaExc;

	            	throw Exception("Aaaa!"_msg, "Fuuuunc!11!1"_func, 1111_line);

                	...
	            }
	            catch (std::exception& exc) // Also catches Exception
	            {
	            	throw Exception("Exception occured!!!", VAEXC_POS, exc);
	            }
	    @endcode
	*/
	class Exception : public std::exception
	{
	private:
		// Variables:
			_errorInfo::ErrorInfo excepts_[_control::MAX_EXC_COUNT];
			size_t excCount_;

		// Helper functions:
			template <class... Args>
			void parseArgs(_wrappers::ArgMsg&& msg, Args&&... args) noexcept;

			template <class... Args>
			void parseArgs(_wrappers::ArgMsgConstexpr&& msg, Args&&... args) noexcept;

			template <class... Args>
			void parseArgs(_wrappers::ArgFilename&& file, Args&&... args) noexcept;

			template <class... Args>
			void parseArgs(_wrappers::ArgFunction&& func, Args&&... args) noexcept;

			template <class... Args>
			void parseArgs(_wrappers::ArgLine&& line, Args&&... args) noexcept;

			template <class... Args>
			void parseArgs(Exception&& exc, Args&&... args) noexcept;

			template <class... Args>
			void parseArgs(std::exception&& exc, Args&&... args) noexcept;

			template <class T, class... Args>
			void parseArgs(T&&, Args&&...) noexcept;

			// Recursion bottom
			void parseArgs() noexcept;

	public:
			template <class... Args>
			explicit Exception(Args&&... args) noexcept;

			virtual ~Exception() = default;

			virtual const char* what() const noexcept override;
	};

	// Definitions:

		// Helper funcs:

		template <class... Args>
		void Exception::parseArgs(_wrappers::ArgMsg&& msg, Args&&... args) noexcept
		{
			excepts_[0].writeInfoWithCaption("\nExplanation: ", msg.msg);

			parseArgs(std::forward<Args>(args)...);
		}

		template <class... Args>
		void Exception::parseArgs(_wrappers::ArgMsgConstexpr&& msg, Args&&... args) noexcept
		{
			excepts_[0].writeInfoWithCaption("\nExplanation: ", msg.msg);

			parseArgs(std::forward<Args>(args)...);
		}

		template <class... Args>
		void Exception::parseArgs(_wrappers::ArgFilename&& file, Args&&... args) noexcept
		{
			excepts_[0].writeInfoWithCaption("\nIn file: ", file.file);

			parseArgs(std::forward<Args>(args)...);
		}

		template <class... Args>
		void Exception::parseArgs(_wrappers::ArgFunction&& func, Args&&... args) noexcept
		{
			excepts_[0].writeInfoWithCaption("\nIn function: ", func.func);

			parseArgs(std::forward<Args>(args)...);
		}

		template <class... Args>
		void Exception::parseArgs(_wrappers::ArgLine&& line, Args&&... args) noexcept
		{
			static char buf[20]; // Thats enough for a number representation

			int written = std::sprintf(buf, "%zu", line.line);

			if (written >= 0) excepts_[0].writeInfoWithCaption("\nIn line: ",            buf);
			else              excepts_[0].writeInfoWithCaption("\nIn line: ", "Unknown line");

			parseArgs(std::forward<Args>(args)...);
		}

		template <class... Args>
		void Exception::parseArgs(Exception&& exc, Args&&... args) noexcept
		{
			for (size_t i = 0, j = excCount_; i < exc.excCount_ && j < _control::MAX_EXC_COUNT; ++i, ++j)
			{
				excepts_[j] = exc.excepts_[i];
			}
			excCount_ += exc.excCount_;

			parseArgs(std::forward<Args>(args)...);
		}

		template <class... Args>
		void Exception::parseArgs(std::exception&& exc, Args&&... args) noexcept
		{
			if (excCount_ < _control::MAX_EXC_COUNT)
			{
				excepts_[excCount_].writeInfoWithCaption("", exc.what());
				++excCount_;
			}

			parseArgs(std::forward<Args>(args)...);
		}

		template <class T, class... Args>
		void Exception::parseArgs(T&&, Args&&...) noexcept
		{
			static_assert(sizeof(T) == 0, "VaExc ctor: Unknown argument type (Expected: ArgMsg, ArgFilename, ArgFunction, ArgLine, Exception, std::exception)");
		}

		void Exception::parseArgs() noexcept {}

	// Ctor:

		template <class... Args>
		Exception::Exception(Args&&... args) noexcept :
			excepts_  (),
			excCount_ (1)
		{
			parseArgs(std::forward<Args>(args)...);
		}

	// What function:

		const char* Exception::what() const noexcept
		{
			// 15 there (vvvv) is the approximate size of string "\n\nCaused by:\n\n"
			static constexpr size_t outputSize = (_control::MAX_INFO_SIZE + 15) * _control::MAX_EXC_COUNT;
			static char output[outputSize + 1];

			// That is the initialisation for output
			output[0] = '\0';

			for (size_t excI = 0, strI = 0; excI < excCount_ && strI < outputSize - _control::NOT_ENOUGH_SPACE_SIZE; ++excI)
			{
				int written = std::sprintf(output + strI, "%s", excepts_[excI].info());

				if (written < 0)
				{
					std::sprintf(output + strI, "%s", _control::NOT_ENOUGH_SPACE);
					break;
				}

				strI += written;

				if (excI + 1 < excCount_)
				{
					written = std::sprintf(output + strI, "\n\nCaused by:\n");

					if (written < 0)
					{
						std::sprintf(output + strI, "%s", _control::NOT_ENOUGH_SPACE);
						break;
					}

					strI += written;
				}
			}

			return output;
		}

	// Really useful interface stuff:
	using ArgMsg = _wrappers::ArgMsg;
	using namespace _literals;
}

/// The define for convinience in Exception creation.
#define VAEXC_POS VaExc::_wrappers::ArgFilename(__FILE__),     \
                  VaExc::_wrappers::ArgFunction(__FUNCTION__), \
                  VaExc::_wrappers::ArgLine    (__LINE__)

#endif /*HEADER_GUARD_VA_EXCEPTION_HPP_INCLUDED*/