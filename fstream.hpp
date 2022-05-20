
#ifndef FILE_STREAM_HPP

#if __has_include(<filesystem>)

#define FILE_STREAM_HPP

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <exception>

#pragma warning(disable : 4996)

#define voidptr(x) (reinterpret_cast<void*>(x))

namespace madeargentum {

    namespace io_open_modes {
        const std::string
            out = "w",
            in = "r",
            add = "a",
            out_read = "w+",
            in_write = "r+",
            add_read = "a+",
            bin_out = "wb",
            bin_in = "rb",
            bin_add = "ab",
            bin_out_read = "w+b",
            bin_in_write = "r+b",
            bin_add_read = "a+b";
    }

    const std::vector<std::string>
        modes{ "w", "r", "a", "w+", "r+", "a+", "wb", "rb", "ab", "w+b", "r+b", "a+b" },
        support_def_write{ "w", "a", "w+", "r+", "a+" },
        support_def_read{ "r", "w+", "r+", "a+" },
        support_bytes_writing{ "wb", "ab", "w+b", "r+b", "a+b" },
        support_bytes_read{ "rb", "w+b", "r+b", "a+b" };

    class file_stream {

    private:
        using File = FILE*;
        using cstring = const std::string;
        using buff_ptr = std::shared_ptr<char>;
        File file;
        size_t chunk;
        buff_ptr buff, all_buff;
        char* all_buff_it;
        bool eof = false;
        char* mode;

        enum class Errors {
            cannot_open_file = 1,
            cannot_create_file,
            cannot_set_buffer,
            cannot_allocate_memory,
            mode_not_support_write,
            mode_not_support_read,
            mode_not_support_bytes_writing,
            mode_not_support_bytes_read,
            cannot_read_char,
            cannot_put_char,
            cannot_read_file,
            cannot_write_file,
            cannot_read_bytes_file,
            cannot_write_bytes_file,
            cannot_unget,
            cannot_unget_char,
            eof_error,
            seek_start_error,
            seek_cur_error,
            seek_end_error,
            set_pos_error,
            get_pos_error,
            cannot_flush_file,
            cannot_close_file,
            invalid_mode,
            io_error,
        };

    public:

        class file_exception : public std::exception {
        private:
            Errors code;

            const std::string get_name() const {
                return "file_stream";
            }

            const std::string get_what(Errors err) const {
                switch (err) {
                case file_stream::Errors::cannot_open_file:
                    return "file does not exists";
                    break;
                case file_stream::Errors::cannot_create_file:
                    return "cannot create file";
                    break;
                case file_stream::Errors::cannot_set_buffer:
                    return "cannot set buffer";
                    break;
                case file_stream::Errors::cannot_allocate_memory:
                    return "cannot allocate memory";
                    break;
                case file_stream::Errors::mode_not_support_write:
                    return "file opening mode does not support writing";
                    break;
                case file_stream::Errors::mode_not_support_read:
                    return "file opening mode does not support reading";
                    break;
                case file_stream::Errors::mode_not_support_bytes_writing:
                    return "file opening mode does not support writing bytes";
                    break;
                case file_stream::Errors::mode_not_support_bytes_read:
                    return "file opening mode does not support reading bytes";
                    break;
                case file_stream::Errors::cannot_read_char:
                    return "cannot read char from file";
                    break;
                case file_stream::Errors::cannot_put_char:
                    return "cannot put char in file";
                    break;
                case file_stream::Errors::cannot_read_file:
                    return "cannot read data from file";
                    break;
                case file_stream::Errors::cannot_write_file:
                    return "cannot write data in file";
                    break;
                case file_stream::Errors::cannot_read_bytes_file:
                    return "cannot read bytes from file";
                    break;
                case file_stream::Errors::cannot_write_bytes_file:
                    return "cannot write bytes in file";
                    break;
                case file_stream::Errors::cannot_unget:
                    return "cannot unget char from buffer";
                    break;
                case file_stream::Errors::cannot_unget_char:
                    return "cannot unget current char";
                    break;
                case file_stream::Errors::eof_error:
                    return "end of file";
                    break;
                case file_stream::Errors::seek_start_error:
                    return "cannot seek from file start to need position";
                    break;
                case file_stream::Errors::seek_cur_error:
                    return "cannot seek from file current cursor position to need position";
                    break;
                case file_stream::Errors::seek_end_error:
                    return "cannot seek from file end to need position";
                    break;
                case file_stream::Errors::set_pos_error:
                    return "cannot set file position to need position";
                    break;
                case file_stream::Errors::get_pos_error:
                    return "cannot get file position";
                    break;
                case file_stream::Errors::cannot_flush_file:
                    return "cannot flush file";
                    break;
                case file_stream::Errors::cannot_close_file:
                    return "cannot close file";
                    break;
                case file_stream::Errors::invalid_mode:
                    return "invalid file open mode";
                    break;
                case file_stream::Errors::io_error:
                    return "file_stream error";
                    break;
                default:
                    return "";
                    break;
                }
            }
        public:

            file_exception() : std::exception() {
                code = Errors::io_error;
            }

            file_exception(Errors code) : std::exception() {
                int c = static_cast<int>(code);
                if (c < 1 || c > static_cast<int>(Errors::io_error)) {
                    std::string s = "invalid error code: ";
                    s += c;
                    s += "\n";
                    throw std::exception(s.c_str());
                }
                this->code = code;
            }

            const std::string name() const {
                return get_name();
            }

            const char* what() const {
                return "Use what_str() to get the error description";
            }

            std::string what_str() const {
                std::stringstream ss;
                ss << "[" << get_name() << "] error code "
                    << static_cast<int>(code) << ": " << get_what(code) << "\n";
                std::string str = ss.str();
                return str;
            }

            const std::string what(errno_t code) const {
                if (code < 1 || code > static_cast<int>(Errors::io_error)) {
                    std::string s = "invalid error code: ";
                    s += code;
                    s += "\n";
                    throw std::exception(s.c_str());
                }
                return get_what(static_cast<Errors>(code));
            }

        };

        file_stream()
            : file(nullptr)
            , chunk(0)
            , all_buff_it(nullptr)
            , mode(nullptr) {}

        file_stream(cstring& filename, cstring& mode = "w", const size_t chunk = 16) {
            open(filename, mode, chunk);
        }

        file_stream(file_stream&& right) noexcept
            : file(nullptr)
            , chunk(0)
            , all_buff_it(nullptr) {

            file = right.file;
            buff = right.buff;
            chunk = right.chunk;
            eof = right.eof;
            all_buff = right.all_buff;
            all_buff_it = right.all_buff_it;
            mode = right.mode;

            right.file = nullptr;
            right.buff.reset();
            right.chunk = 0;
            right.eof = false;
            right.all_buff.reset();
            right.all_buff_it = nullptr;
            right.mode = nullptr;
        }

        ~file_stream() {
            if (file)
                fclose(file);
        }

        file_stream& operator=(file_stream&& right) noexcept {
            if (this != &right) {
                file = right.file;
                buff = right.buff;
                chunk = right.chunk;
                eof = right.eof;
                all_buff = right.all_buff;
                all_buff_it = right.all_buff_it;
                mode = right.mode;

                right.file = nullptr;
                right.buff.reset();
                right.chunk = 0;
                right.eof = false;
                right.all_buff.reset();
                right.all_buff_it = nullptr;
                right.mode = nullptr;
            }
            return *this;
        }

        void open(cstring& filename, cstring& mode = "w", const size_t chunk = 16) {
            if (std::find(modes.begin(), modes.end(), mode) == modes.end()) {
                throw file_exception(Errors::invalid_mode);
            }
            this->mode = const_cast<char*>(mode.c_str());
            file = fopen(filename.c_str(), mode.c_str());
            if (!file) {
                if (mode.find("w") || mode.find("a+"))
                    if (std::filesystem::exists(filename))
                        throw file_exception(Errors::cannot_open_file);
                    else
                        throw file_exception(Errors::cannot_create_file);
                else if (mode.find("r"))
                    throw file_exception(Errors::cannot_open_file);
            }
            else
                if (setvbuf(file, buff.get(), _IOFBF, chunk))
                    throw file_exception(Errors::cannot_set_buffer);
            this->chunk = chunk;
            buff = buff_ptr(new char[chunk]);
            all_buff = buff_ptr(new char[std::filesystem::file_size(filename)]);
            all_buff_it = all_buff.get();
        }

        void close() {
            if (file)
                if (fclose(file) == EOF) throw file_exception(Errors::cannot_close_file);
        }

        void set_chunk(size_t chunk) {
            this->chunk = chunk;
            buff = std::shared_ptr<char>(new char[chunk]);
        }

        void flush() {
            if (fflush(file) == EOF)
                throw file_exception(Errors::cannot_flush_file);
        }

        int getchar() {
            if (std::find(support_def_read.begin(), support_def_read.end(),
                const_cast<const char*>(mode)) == support_def_read.end()) {
                throw file_exception(Errors::mode_not_support_read);
            }
            if (!file)
                throw file_exception(Errors::cannot_read_char);
            if (eof)
                throw file_exception(Errors::eof_error);
            int c = getc(file);
            eof = c == EOF;
            *all_buff_it++ = c;
            return c;
        }

        void putchar(int c) {
            if (std::find(support_def_write.begin(), support_def_write.end(),
                const_cast<const char*>(mode)) == support_def_write.end()) {
                throw file_exception(Errors::mode_not_support_write);
            }
            if (!file)
                throw file_exception(Errors::cannot_read_char);
            if (putc(c, file) == EOF)
                throw file_exception(Errors::eof_error);
        }

        void unget() {
            if (!all_buff_it || all_buff_it < all_buff.get())
                throw file_exception(Errors::cannot_unget);
            if (ungetc(*all_buff_it, file) == EOF)
                throw file_exception(Errors::cannot_unget_char);
            all_buff_it--;
        }

        void ungetchar(int c) {
            if (ungetc(c, file) == EOF)
                throw file_exception(Errors::cannot_unget_char);
        }

        std::string readline() {
            std::string res;
            char c;
            for (c = getchar(); c != '\n' && c != EOF; c = getchar()) {
                res += c;
            }
            if (c == EOF) eof = true;
            return res;
        }

        std::string read() {
            std::string res;
            while (!eof) {
                res += readline();
            }
            return res;
        }

        std::string read_chunky(size_t chunk = 0) {
            if (std::find(support_bytes_read.begin(), support_bytes_read.end(),
                const_cast<const char*>(mode)) == support_bytes_read.end()) {
                throw file_exception(Errors::mode_not_support_bytes_read);
            }
            if (!chunk) chunk = this->chunk;
            std::shared_ptr<char> buff = std::shared_ptr<char>(new char[chunk]);
            if (fread(buff.get(), 1, chunk, file) != chunk) {
                if (feof(file))
                    eof = true;
                else
                    throw file_exception(Errors::cannot_read_bytes_file);
            }
            return std::string(buff.get());

        }

        template <class T>
        T* deserialize(size_t count = 1) {
            if (std::find(support_bytes_read.begin(), support_bytes_read.end(),
                const_cast<const char*>(mode)) == support_bytes_read.end()) {
                throw file_exception(Errors::mode_not_support_bytes_read);
            }
            T* tmp = (T*)malloc(sizeof(T) * count);
            if (!tmp) {
                throw file_exception(Errors::cannot_allocate_memory);
                return nullptr;
            }
            if (fread(voidptr(tmp), sizeof(T), count, file) != count)
                throw file_exception(Errors::cannot_read_bytes_file);
            return tmp;
        }

        template <class T>
        void deserialize(const T& obj, size_t count = 1) {
            if (std::find(support_bytes_read.begin(), support_bytes_read.end(),
                const_cast<const char*>(mode)) == support_bytes_read.end()) {
                throw file_exception(Errors::mode_not_support_bytes_read);
            }
            if (fread(voidptr(&obj), sizeof(T), count, file) != count)
                throw file_exception(Errors::cannot_read_bytes_file);
        }

        void writeline(cstring& data, char end = '\n') {
            for (char c : data) {
                if (c == '\n') break;
                putchar(c);
            }
            if (end) putchar(end);
        }

        void write(cstring& data, char end = '\n') {
            for (char c : data) {
                putchar(c);
            }
            if (end) putchar(end);
        }

        void write_chunky(cstring& data, size_t chunk = 0) {
            if (std::find(support_bytes_writing.begin(), support_bytes_writing.end(),
                const_cast<const char*>(mode)) == support_bytes_writing.end()) {
                throw file_exception(Errors::mode_not_support_bytes_writing);
            }
            if (!chunk) chunk = this->chunk;
            if (fwrite(data.c_str(), 1, chunk, file) != chunk)
                throw file_exception(Errors::cannot_write_bytes_file);
        }

        template <class T>
        void serialize(const T& obj, size_t count = 1) {
            if (std::find(support_bytes_writing.begin(), support_bytes_writing.end(),
                const_cast<const char*>(mode)) == support_bytes_writing.end()) {
                throw file_exception(Errors::mode_not_support_bytes_writing);
            }
            if (fwrite(voidptr(&obj), sizeof(T), count, file) != count)
                throw file_exception(Errors::cannot_write_bytes_file);
        }

        void seek(long pos) {
            if (fseek(file, pos, SEEK_SET))
                throw file_exception(Errors::seek_start_error);
        }

        void seek_cursor(long pos) {
            if (fseek(file, pos, SEEK_CUR))
                throw file_exception(Errors::seek_cur_error);
        }

        void seek_end(long pos) {
            if (fseek(file, pos, SEEK_END))
                throw file_exception(Errors::seek_end_error);
        }

        long tell() {
            return ftell(file);
        }

        void set_pos(fpos_t pos) {
            if (fsetpos(file, &pos))
                throw file_exception(Errors::set_pos_error);
        }

        fpos_t get_pos() {
            fpos_t pos = 0;
            if (fgetpos(file, &pos))
                throw file_exception(Errors::get_pos_error);
            return pos;
        }
    };

}

#undef voidptr

#else

#error The <filesystem> module cannot be found (available from C++17)

#endif  // __has_include(<filesystem>)

#endif	// FILE_STREAM_HPP