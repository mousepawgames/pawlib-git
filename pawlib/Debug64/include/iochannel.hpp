/**PawLIB: IOChannel
* Allows managed, custom output to any console or terminal.
* @since 1.0
* @author Jason C. McDonald
*/

/*
WHAT IS IOCHANNEL?
IOChannel is intended both as a replacement and wrapper for `std::iostream` and
`stdio.h/printf`. It allows for messages and errors to be output to multiple
sources simultaneously and asyncronously using signals (libsigc++). New
messages from any source are added to the back of the queue, and arbitrary
outputs can read them asynchronously from the front, either destructively or
non-destructively.

Each output is also able to individually timing, as well as which messages it is
interested in and how it reads them, without interfering with the behavior
of other outputs.

EXTERNAL OUTPUTS
An external output waits for a signal to be dispatched before it collects its
messages. Different signals are dispatched for different levels of verbosity
and categories.

INTERNAL OUTPUTS
Optionally, iochannel can output to the terminal automatically via either
`std::iostream` or `stdio.h/printf`. This output can be controlled externally.
For example, a developer might choose to create pseudocommands in their
command-line that allow them to change verbosity on-the-fly while the program
is running.

VERBOSITY
The concept of verbosity allows for developers to write and leave all manner
of useful output data, including debug information, detailed error messages,
and outright snark. Verbosity can be toggled globally for a channel,
or on a connection-by-connection basis.

Verbosity ranges from 0 (only essential messages) to 3 (literally all messages).

CATEGORY
Messages can be assigned a category, which makes it easier to different messages
to be sent to different outputs, or otherwise be handled differently. At the
moment, the categories are...
 * Normal Messages
 * Warnings
 * Errors
 * Debug Output

CROSS-PLATFORM FORMATTING
IOChannel offers coloring and basic formatting on both UNIX and Windows systems
via the same interface.

*/

#ifndef PAWLIB_IOCHANNEL_H
#define PAWLIB_IOCHANNEL_H

//Needed for output sources.
#include <stdio.h>
#include <iostream>

//Needed for the `intptr_t` type
#include <cstdint>

#include <typeinfo>

//TODO: Swap to pawlib::flexarray
#include <vector>

//Import what we need from sigc++
#include <sigc++/signal.h>
#include <sigc++/trackable.h>

/*We are only using std::string and std::queue temporarily.
These need to be swapped out for pawlib alternatives ASAP.*/
#include <string>
#include <queue>

//We use C's classes often.
#include <cstdio>

#include <stdutils.h>

namespace pawlib
{
    namespace ioformat
    {
        enum IOFormatBase
        {
            base_bin = 2,
            base_2 = 2,
            base_ter = 3,
            base_3 = 3,
            base_quat = 4,
            base_4 = 4,
            base_quin = 5,
            base_5 = 5,
            base_sen = 6,
            base_6 = 6,
            base_sep = 7,
            base_7 = 7,
            base_oct = 8,
            base_8 = 8,
            base_9 = 9,
            base_dec = 10,
            base_10 = 10,
            base_und = 11,
            base_11 = 11,
            base_duo = 12,
            base_doz = 12,
            base_12 = 12,
            base_tri = 13,
            base_13 = 13,
            base_tetra = 14,
            base_14 = 14,
            base_pent = 15,
            base_15 = 15,
            base_hex = 16,
            base_16 = 16,
            base_17 = 17,
            base_18 = 18,
            base_19 = 19,
            base_vig = 20,
            base_20 = 20,
            base_21 = 21,
            base_22 = 22,
            base_23 = 23,
            base_24 = 24,
            base_25 = 25,
            base_26 = 26,
            base_27 = 27,
            base_28 = 28,
            base_29 = 29,
            base_30 = 30,
            base_31 = 31,
            base_32 = 32,
            base_33 = 33,
            base_34 = 34,
            base_35 = 35,
            base_36 = 36
        };

        //PRECISION
        struct set_precision
        {
            set_precision(int p):precision(p){}
            int precision = 14;
        };

        enum IOFormatSciNotation
        {
            ///Turn off all scientific notation.
            sci_none = 0,
            ///Automatically select the best option.
            sci_auto = 1,
            ///Turn on all scientific notation.
            sci_on = 2
        };

        enum IOFormatPointer
        {
            ///Print the value at the address.
            ptr_value = 0,
            ///Print the actual memory address.
            ptr_address = 1,
            ///Dump the hexadecimal representation of the memory at address.
            ptr_memory = 2
        };

        /**Indicate how many bytes to read from any pointer that isn't
         * recognized explicitly by iochannel, including void pointers.
         * This will not override the memory dump read size of built-in types.*/
        struct read_size
        {
            /**Indicate how many bytes to read from any pointer that isn't
             * recognized explicitly by iochannel, including void pointers.
             * This will not override the memory dump read size of built-in
             * types.
             * CAUTION: Misuse can cause SEGFAULT or other memory errors.
             \param the number of bytes to read*/
            read_size(unsigned int i):readsize(i){}
            unsigned int readsize = 1;
        };

        enum IOFormatMemorySeperators
        {
            ///Output as one long string.
            mem_nosep = 0,
            mem_bytesep = (1 << 0),
            mem_wordsep = (1 << 1),
            mem_allsep = 3
        };

        enum IOFormatNumeralCase
        {
            ///Print all non-numeral digits as lowercase.
            num_lower = 0,
            ///Print all non-numeral digits as uppercase.
            num_upper = 1
        };

        /**The standard ANSI text attributes.*/
        enum IOFormatTextAttributes
        {
            ///Turn of all attributes.
            ta_none = 0,
            ///Bold text.
            ta_bold = 1,
            ///Underlined text.
            ta_underline = 4,
            ///Inverted text colors, also known as "reverse video".
            ta_invert = 6
        };

        /**The standard ANSI text foreground colors.*/
        enum IOFormatTextFG
        {
            //None.
            fg_none = 0,
            ///Black text.
            fg_black = 30,
            ///Red text.
            fg_red = 31,
            ///Green text
            fg_green = 32,
            ///Yellow text.
            fg_yellow = 33,
            ///Blue text.
            fg_blue = 34,
            ///Magenta text.
            fg_magenta = 35,
            ///Cyan text.
            fg_cyan = 36,
            ///White text.
            fg_white = 37
        };

        /**The standard ANSI text background colors.*/
        enum IOFormatTextBG
        {
            //None.
            bg_none = 0,
            ///Black text background.
            bg_black = 40,
            ///Red text background.
            bg_red = 41,
            ///Green text background.
            bg_green = 42,
            ///Yellow text background.
            bg_yellow = 43,
            ///Blue text background.
            bg_blue = 44,
            ///Magenta text background.
            bg_magenta = 45,
            ///Cyan text background.
            bg_cyan = 46,
            ///White text background.
            bg_white = 47
        };

        /**The level of verbosity necessary for the message to display.*/
        enum IOFormatVerbosity
        {
            /**Only essential messages and errors. For normal end-use.
            Shipping default.*/
            vrb_quiet = 0,
            /**Common messages and errors. For common and normal end-user
            testing.*/
            vrb_normal = 1,
            /**Most messages and errors. For detailed testing and
            debugging.*/
            vrb_chatty = 2,
            /**Absolutely everything. For intense testing, detailed
            debugging, and driving the developers crazy.*/
            vrb_tmi = 3
        };

        /**The category of the message. Don't confuse this with Verbosity!
        Both a loop iterator output (V_TMI) and rare function start
        notification (V_NORMAL or V_CHATTY) belong under C_DEBUG, but they
        have different verbosity levels, as demonstrated.*/
        enum IOFormatCategory
        {
            /**The default value - anything that doesn't fit elsewhere.*/
            cat_normal = 0,
            /**Warnings, but not necessarily errors.*/
            cat_warning = 1,
            /**Error messages.*/
            cat_error = 2,
            /**Debug messages, such as variable outputs.*/
            cat_debug = 3,
            /**All message categories. Does not have a correlating signal.*/
            cat_all = 4
        };

        /**Special structures for iochannel, such as "END".*/
        enum IOSpecial
        {
            /**End of message, remove formatting.*/
            io_end = 0
        };

        enum IOEchoMode
        {
            echo_none = 0,
            echo_printf = 1,
            echo_cout = 2
        };
    }

    //We need this here to shorten code in the following class/classes.
    using namespace ioformat;

    /**An iochannel allows console output to be custom routed to one or more
    * text-based output channels, including the terminal. It supports various
    * advanced functions, formatting and colors, and message priority.*/
    //TODO: Waiting on StackOverflow.
    class iochannel : public sigc::trackable
//    class iochannel
    {
        public:
            /**Declares a new iochannel instance.*/
            iochannel();

            /**Emitted when a message with verbosity 0 (quiet) is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_v_quiet;
            /**Emitted when a message with verbosity <= 1 (normal) is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_v_normal;
            /**Emitted when a message with verbosity <=2 (chatty) is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_v_chatty;
            /**Emitted when a message with verbosity <=3 (tmi) is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_v_tmi;

            /**Emitted when a message with category "normal" is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_c_normal;
            /**Emitted when a message with category "warning" is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_c_warning;
            /**Emitted when a message with category "error" is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_c_error;
            /**Emitted when a message with category "debug" is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_c_debug;

            /**Emitted when any message is broadcast.
            Callback must be of form 'void callback(char*){}'*/
            sigc::signal<void, std::string> signal_all;

            /*C DATA TYPES*/
            iochannel& operator<<(const bool&);
            iochannel& operator<<(const int&);
            iochannel& operator<<(const float&);
            iochannel& operator<<(const double&);
            iochannel& operator<<(const char&);

            /*C DATA TYPE POINTERS*/
            iochannel& operator<<(const bool*);
            iochannel& operator<<(const int*);
            iochannel& operator<<(const float*);
            iochannel& operator<<(const double*);
            iochannel& operator<<(const char*);
            iochannel& operator<<(const void*);

            /*C++ DATA TYPES*/
            iochannel& operator<<(const std::string&);

            /*C++ DATA TYPE POINTERS*/
            iochannel& operator<<(const std::string*);

            /*MATH FLAGS*/
            iochannel& operator<<(const IOFormatBase&);
            iochannel& operator<<(const set_precision&);
            iochannel& operator<<(const IOFormatSciNotation&);
            iochannel& operator<<(const IOFormatNumeralCase&);
            iochannel& operator<<(const IOFormatPointer&);
            iochannel& operator<<(const read_size&);
            iochannel& operator<<(const IOFormatMemorySeperators&);

            /*FORMAT FLAGS*/
            iochannel& operator<<(const IOFormatTextBG&);
            iochannel& operator<<(const IOFormatTextFG&);
            iochannel& operator<<(const IOFormatTextAttributes&);

            /*BROADCAST FLAGS*/
            iochannel& operator<<(const IOFormatCategory&);
            iochannel& operator<<(const IOFormatVerbosity&);
            iochannel& operator<<(const IOSpecial&);

            void configure_echo(IOEchoMode, IOFormatVerbosity = vrb_tmi, IOFormatCategory = cat_all);

            ~iochannel();
        protected:
            //TODO: Swap for pawlib::string
            std::string msg;

            IOEchoMode echomode = echo_printf;
            IOFormatVerbosity echovrb = vrb_tmi;
            IOFormatCategory echocat = cat_all;

            /*Current settings set by enums and flags. These change as we go,
            and should be reset after each message.*/
            IOFormatBase base = base_10;
            int precision = 14;
            IOFormatSciNotation sci = sci_auto;
            IOFormatNumeralCase numcase = num_lower;
            IOFormatPointer ptr = ptr_value;
            unsigned int readsize = 1;
            unsigned char memformat = 3;

            IOFormatTextAttributes ta = ta_none;
            IOFormatTextBG bg = bg_none;
            IOFormatTextFG fg = fg_none;

            IOFormatVerbosity vrb = vrb_normal;
            IOFormatCategory cat = cat_normal;

            //The string containing the format.
            std::string format = "";
            //TODO: Swap for pawlib::string

            /**Insert a C string into the output stream. Automatically applies
            * unapplied attributes before inserting text.
            * \param the string to insert.
            * \param whether the call was recursive. (Internal use only!)
            */
            void inject(std::string, bool=false);

            /**Insert a memory address or its raw contents into the output
            * stream.
            * \param the address to insert
            * \param the size of the object referenced
            * \param whether to print literal address (false) or memory
            * dump (true); default false*/
            void inject(const void*, unsigned int len, bool=false);

            /**Transmit the current pending output stream and reset in
            * preparation for the next message.
            */
            void transmit();

            ///Dirty flag raised when attributes are changed and not yet applied.
            bool dirty_attributes = false;

            /**Apply formatting attributes (usually ANSI) that are pending.
            * \param true if new attributes were applied
            */
            bool apply_attributes();

            /**Clear the channel's message substring array.*/
            void clear_msg();
            //TODO: Kill

            /**Reset all attributes.*/
            void reset_attributes();

            /**Reset all flags and attributes.*/
            void reset_flags();

    };
}


#endif // PAWLIB_BROADCAST_H
