#ifndef __BSPATCH_HPP__
#define __BSPATCH_HPP__

#include "File.hpp"
#include <memory>
#include <bzlib.h>

namespace atcupdateservice {
namespace utils {

class BspatchStream
{
public:
    typedef std::shared_ptr<BspatchStream> ptr;
    BspatchStream(const std::string& file)
        : m_name(file), m_newSize(0) {}
	virtual int read(uint8_t *data, uint32_t size) = 0;
    virtual int newSize() {
        return m_newSize;
    }
    const std::string &getName() const {
        return m_name;
    }
    virtual ~BspatchStream() {}
protected:
    std::string m_name;
    int64_t m_newSize;
};

class Bz2BspatchStream : public BspatchStream {
public:
    typedef std::shared_ptr<Bz2BspatchStream> ptr;
public:
    ~Bz2BspatchStream();
    Bz2BspatchStream(const std::string& file);
    int read(uint8_t *data, uint32_t size) override;
private:
    BZFILE *m_bz2;
    FILE *m_patch;
};

#define BSPATCH_DEFAULT_LIMIT   (64 * 1024)

class Bspatch : public std::enable_shared_from_this<Bspatch> {
public:
    typedef std::shared_ptr<Bspatch> ptr;
public:
    static  Bspatch::ptr create() {
        auto patch = Bspatch::ptr(new Bspatch());
        patch->m_limit = BSPATCH_DEFAULT_LIMIT;

        return patch;
    }

    Bspatch::ptr setSource(File::ptr file) {
        m_source = file;
        return shared_from_this();
    }
    Bspatch::ptr setDestiny(File::ptr file) {
        m_dest = file;
        return shared_from_this();
    }
    Bspatch::ptr setPatch(BspatchStream::ptr stream) {
        m_stream = stream;
        return shared_from_this();
    }
    Bspatch::ptr configBufferLimit(uint64_t limit) {
        m_limit = limit / 2;
        return shared_from_this();
    }
    int patch();
private:
    Bspatch() {};
private:
    File::ptr m_source;
    File::ptr m_dest;
    BspatchStream::ptr m_stream;
    uint64_t m_limit;
};

}
}

#endif