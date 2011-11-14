#!/usr/bin/python

import sys

import ctypes
import ctypes.util
from ctypes import c_int, c_uint, c_uint8, c_size_t, c_int64, c_uint32
from ctypes import c_double, c_float
from ctypes import c_void_p, c_char_p, c_bool

# create our library pointers
# FIXME cross-platform loading
rs = ctypes.CDLL("libredstone.dylib")
c = ctypes.CDLL(ctypes.util.find_library("c"))

# some useful c functions
c.fdopen.restype = c_void_p
c.fdopen.argtypes = [c_int, c_char_p]
c.fclose.restype = None
c.fclose.argtypes = [c_void_p]

# first up, a C file compatibility class
class CFile(object):
    """This class wraps a python file object so that it can be passed
    in to C functions that require a FILE *."""
    def __init__(self, f):
        self._as_parameter_ = c.fdopen(f.fileno(), f.mode)
    #def __del__(self):
        #if not c is None:
        #    c.fclose(self._as_parameter_)

##
## Metaclass and Base Classes
##

class RedstoneMetaclass(type):
    """This metaclass searches for method signatures in
    Class.Methods.methname, and looks up the corresponding rs_class_*
    method, sets the signature, then puts it into the main class. For
    example: 
    
    class Example:
        __metaclass__ = RedstoneMetaclass
        class Methods:
            method_name = (None, [c_void_p, c_char_p])
        class Properties:
            prop = (c_void_p, NBT, lambda ptr: NBT(ptr))
    
    will create a class where Example._method_name maps directly to
    rs_example_method_name, set up to take a void* and a char* and
    return void. Also, it will create a property prop, with get_prop
    and set_prop using c-type c_void_p, python type NBT, and
    converting from C to python using NBT(ptr).
    """
    
    def __new__(cls, name, bases, dct):
        if not 'Methods' in dct:
            return super(RedstoneMetaclass, cls).__new__(cls, name, bases, dct)
        
        m = dct['Methods']
        p = dct.get('Properties', object())
        prefix = 'rs_' + getattr(m, '_prefix_', name.lower()) + '_'
        del dct['Methods']
        if 'Properties' in dct:
            del dct['Properties']
        obj = super(RedstoneMetaclass, cls).__new__(cls, name, bases, dct)
        
        for methname in dir(m):
            if methname.startswith('__'):
                continue
            res, args = getattr(m, methname)
            methname = methname.rstrip('_')
            meth = getattr(rs, prefix + methname)
            
            meth.restype = res
            meth.argtypes = args
            
            setattr(obj, '_' + methname, meth)
        for propname in dir(p):
            if propname.startswith('__'):
                continue
            try:
                ctype, pytype, convert, check = getattr(p, propname)
            except ValueError:
                ctype, pytype, convert = getattr(p, propname)
                check = lambda o: True
            propname = propname.rstrip('_')
            getmeth = getattr(rs, prefix + 'get_' + propname)
            setmeth = getattr(rs, prefix + 'set_' + propname)
            
            if not convert:
                convert = lambda s: s
            
            getmeth.restype = ctype
            getmeth.argtypes = [c_void_p]
            setmeth.restype = None
            setmeth.argtypes = [c_void_p, ctype]
            
            def gen_functions(propname, getmeth, setmeth, ctype, pytype, convert, check):
                def get_prop(self):
                    if not check(self):
                        raise TypeError("could not get %s, %s is incorrect type" % (propname, obj.__name__))
                    return convert(getmeth(self))
                def set_prop(self, val):
                    if not check(self):
                        raise TypeError("could not set %s, %s is incorrect type" % (propname, obj.__name__))
                    if not isinstance(val, pytype):
                        raise TypeError("could not set %s, value not a %s" % (propname, pytype.__name__))
                    setmeth(self, val)
                return get_prop, set_prop
            
            get_prop, set_prop = gen_functions(propname, getmeth, setmeth, ctype, pytype, convert, check)
            
            setattr(obj, 'get_' + propname, get_prop)
            setattr(obj, 'set_' + propname, set_prop)
            setattr(obj, propname, property(get_prop, set_prop))
        return obj

class RedstoneObject(object):
    """This is a superclass for all simple libredstone objects. It
    stores the pointer provided so that you may pass instances of this
    class directly to ctypes-bound functions. It automatically calls
    the function named in _destructor_ when the object is freed;
    this defaults to self._free."""
    __metaclass__ = RedstoneMetaclass
    _destructor_ = '_free'
    def __init__(self, ptr):
        self._as_parameter_ = ptr
    def __del__(self):
        destructor = getattr(self, self._destructor_, None)
        if destructor: destructor(self)
    def __repr__(self):
        return "<%s.%s : 0x%x>" % (__name__, self.__class__.__name__, self._as_parameter_)
    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self._as_parameter_ == other._as_parameter_
        else:
            return False
    def __ne__(self, other):
        return not self.__eq__(other)
    def __hash__(self):
        if hasattr(self._as_parameter_, 'value'):
            return hash(self._as_parameter_.value)
        else:
            return hash(self._as_parameter_)
    def __nonzero__(self):
        return bool(self._as_parameter_)

class RedstoneCountedObject(object):
    """This is a superclass for all reference-counted libredstone
    objects. It behaves like RedstoneObject, except it will ref an
    object on creation if an extra init parameter is passed as
    True. The functions named in _managers_ will be called for ref
    and unref; this defaults to _managers_ = ('_ref', '_unref'),
    calling self._ref and self._unref."""
    __metaclass__ = RedstoneMetaclass
    _managers_ = ('_ref', '_unref')
    def __init__(self, ptr, unowned=False):
        self._as_parameter_ = ptr
        if unowned:
            ref = getattr(self, self._managers_[0], None)
            if ref: ref(self)
    def __del__(self):
        unref = getattr(self, self._managers_[1], None)
        if unref: unref(self)
    def __repr__(self):
        return "<%s.%s : 0x%x>" % (__name__, self.__class__.__name__, self._as_parameter_)
    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self._as_parameter_ == other._as_parameter_
        else:
            return False
    def __ne__(self, other):
        return not self.__eq__(other)
    def __hash__(self):
        if hasattr(self._as_parameter_, 'value'):
            return hash(self._as_parameter_.value)
        else:
            return hash(self._as_parameter_)
    def __nonzero__(self):
        return bool(self._as_parameter_)

##
## memory.h
##

# we don't wrap these officially, but we use them internally
rs.rs_malloc.restype = c_void_p
rs.rs_malloc.argtypes = [c_size_t]
rs.rs_free.restype = None
rs.rs_free.argtypes = [c_void_p]

##
## compression.h
##

AUTO_COMPRESSION, GZIP, ZLIB, UNKNOWN_COMPRESSION = range(4)

rs.rs_decompress.restype = None
rs.rs_decompress.argtypes = [c_int, c_void_p, c_size_t, c_void_p, c_void_p]
rs.rs_compress.restype = None
rs.rs_compress.argtypes = [c_int, c_void_p, c_size_t, c_void_p, c_void_p]
rs.rs_get_compression_type.restype = c_int
rs.rs_get_compression_type.argtypes = [c_void_p, c_size_t]

def _compress_helper(func, err, enc, indata):
    inbuf = ctypes.create_string_buffer(indata)
    outptr = c_void_p(0)
    outlen = c_size_t(0)
    
    func(enc, inbuf, ctypes.sizeof(inbuf), ctypes.byref(outptr), ctypes.byref(outlen))
    
    if not outptr.value:
        raise RuntimeError(err)
    ret = ctypes.string_at(outptr, outlen.value)
    rs.rs_free(outptr)
    return ret

def compress(enc, rawdata):
    return _compress_helper(rs.rs_compress, "could not compress string", enc, rawdata)
def decompress(enc, gzdata):
    return _compress_helper(rs.rs_decompress, "could not decompress string", enc, gzdata)

def get_compression_type(data):
    inbuf = ctypes.create_string_buffer(data)
    return rs.rs_get_compression_type(inbuf, ctypes.sizeof(inbuf))

##
## tag.h
##

TAG_END, TAG_BYTE, TAG_SHORT, TAG_INT, TAG_LONG, TAG_FLOAT, TAG_DOUBLE, TAG_BYTE_ARRAY, TAG_STRING, TAG_LIST, TAG_COMPOUND = range(11)

class Tag(RedstoneCountedObject):
    class Methods:
        new0 = (c_void_p, [c_int])
        get_type = (c_int, [c_void_p])
        # new, newv are C-conveniences, we don't need them
        ref = (None, [c_void_p])
        unref = (None, [c_void_p])
        
        find = (c_void_p, [c_void_p, c_char_p])
        print_ = (None, [c_void_p, c_void_p])
        pretty_print = (None, [c_void_p, c_void_p])
        
        get_byte_array = (c_void_p, [c_void_p])
        get_byte_array_length = (c_uint32, [c_void_p])
        set_byte_array = (None, [c_void_p, c_uint32, c_void_p])
        
        # iterators are another c convenience, we do them differently
        # see __iter__
        list_iterator_init = (None, [c_void_p, c_void_p])
        list_iterator_next = (c_bool, [c_void_p, c_void_p])
        list_get_type = (c_int, [c_void_p])
        list_set_type = (None, [c_void_p, c_int])
        list_get_length = (c_uint32, [c_void_p])
        list_get = (c_void_p, [c_void_p, c_uint32])
        list_delete = (None, [c_void_p, c_uint32])
        list_insert = (None, [c_void_p, c_uint32, c_void_p])
        list_reverse = (None, [c_void_p])
        
        # again, iterators are done differently. See __iter__
        compound_iterator_init = (None, [c_void_p, c_void_p])
        compound_iterator_next = (c_bool, [c_void_p, c_void_p, c_void_p])
        compound_get_length = (c_uint32, [c_void_p])
        compound_get = (c_void_p, [c_void_p, c_char_p])
        # get_chain[v] is a convenience, tag['key'] is preferred
        compound_set = (None, [c_void_p, c_char_p, c_void_p])
        compound_delete = (None, [c_void_p, c_char_p])
    class Properties:
        integer = (c_int64, (int, long), None, lambda o: o.type in [TAG_BYTE, TAG_SHORT, TAG_INT, TAG_LONG])
        float_ = (c_double, (int, long, float), None, lambda o: o.type in [TAG_FLOAT, TAG_DOUBLE])
        string = (c_char_p, str, None, lambda o: o.type == TAG_STRING)
    
    @classmethod
    def new0(cls, typ):
        return cls(cls._new0(typ), True)
    
    @classmethod
    def new(cls, value, typ=-1):
        if typ == -1:
            # auto-detect type
            if type(value) == int:
                typ = TAG_INT
            elif type(value) == long:
                typ = TAG_LONG
            elif type(value) == float:
                typ = TAG_DOUBLE
            elif isinstance(value, str):
                typ = TAG_STRING
            elif isinstance(value, list):
                typ = TAG_LIST
            elif isinstance(value, dict):
                typ = TAG_COMPOUND
            else:
                raise TypeError("invalid type for Tag")
        t = cls.new0(typ)
        if t:
            t.value = value
        return t
    
    def get_type(self):
        return self._get_type(self)
    type = property(get_type)
    
    def find(self, name):
        ptr = self._find(self, name)
        if not ptr:
            raise RuntimeError("could not find tag: %s" % (name,))
        return Tag(ptr, True)
    
    def print_(self, f=sys.stdout):
        f = CFile(f)
        self._print(self, f)
    
    def pretty_print(self, f=sys.stdout):
        f = CFile(f)
        self._pretty_print(self, f)
    
    def get_byte_array_length(self):
        if not self.get_type() == TAG_BYTE_ARRAY:
            raise TypeError("Tag is not a byte array")
        return self._get_byte_array_length(self)
    byte_array_length = property(get_byte_array_length)
    def get_byte_array(self):
        if not self.get_type() == TAG_BYTE_ARRAY:
            raise TypeError("Tag is not a byte array")
        ptr = self._get_byte_array(self)
        if not ptr:
            return None
        return ctypes.string_at(ptr, self.get_byte_array_length())
    def set_byte_array(self, value):
        if not self.get_type() == TAG_BYTE_ARRAY:
            raise TypeError("Tag is not a byte array")
        inbuf = ctypes.create_string_buffer(value)
        self._set_byte_array(self, ctypes.sizeof(inbuf), inbuf)
    byte_array = property(get_byte_array, set_byte_array)
        
    def list_get_type(self):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        return self._list_get_type(self)
    def list_set_type(self, typ):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        if self.list_get_length() > 0:
            raise RuntimeError("cannot set the type of a non-empty list")
        self._list_set_type(self, typ)
    list_type = property(list_get_type, list_set_type)
    
    def list_get_length(self):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        return self._list_get_length(self)
    list_length = property(list_get_length)
    
    def list_get(self, i):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        ptr = self._list_get(self, i)
        if not ptr:
            raise IndexError('list index out of range')
        return Tag(ptr, True)
    def list_delete(self, i):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        self._list_delete(self, i)
    def list_insert(self, i, tag):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        if not isinstance(tag, Tag):
            raise ValueError("cannot insert object, it's not a Tag")
        self._list_insert(self, i, tag)
    def list_reverse(self):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        self._list_reverse(self)
        
    def compound_get_length(self):
        if not self.get_type() == TAG_COMPOUND:
            raise TypeError("Tag is not a compound object")
        return self._compound_get_length(self)
    compound_length = property(compound_get_length)
    
    def compound_get(self, key):
        if not self.get_type() == TAG_COMPOUND:
            raise TypeError("Tag is not a compound object")
        ptr = self._compound_get(self, key)
        if not ptr:
            raise KeyError(key)
        return Tag(ptr, True)
    def compound_set(self, key, value):
        if not self.get_type() == TAG_COMPOUND:
            raise TypeError("Tag is not a compound object")
        if not isinstance(value, Tag):
            raise TypeError("value is not a Tag")
        self._compound_set(self, key, value)
    def compound_delete(self, key):
        if not self.get_type() == TAG_COMPOUND:
            raise TypeError("Tag is not a compound object")
        self._compound_delete(self, key)
    
    # list / compound / array pythonic conveniences
    def __len__(self):
        typ = self.get_type()
        if typ == TAG_LIST:
            return self.list_get_length()
        elif typ == TAG_COMPOUND:
            return self.compound_get_length()
        elif typ == TAG_BYTE_ARRAY:
            return self.get_byte_array_length()
        else:
            raise TypeError("Tag type does not have a len()")
    def __getitem__(self, key):
        typ = self.get_type()
        if typ == TAG_LIST:
            if key < 0:
                key += self.list_get_length()
            return self.list_get(key)
        elif typ == TAG_COMPOUND:
            return self.compound_get(key)
        else:
            raise TypeError("Tag type cannot be indexed")
    def __setitem__(self, key, value):
        if not isinstance(value, Tag):
            raise TypeError("Cannot add non-Tag values to Tags")
        typ = self.get_type()
        if typ == TAG_LIST:
            if key < 0:
                key += self.list_get_length()
            self.list_delete(key)
            self.list_insert(key, value)
        elif typ == TAG_COMPOUND:
            self.compound_set(key, value)
        else:
            raise TypeError("Tag type cannot be indexed")
    def __delitem__(self, key):
        typ = self.get_type()
        if typ == TAG_LIST:
            if key < 0:
                key += self.list_get_length()
            self.list_delete(key)
        elif typ == TAG_COMPOUND:
            self.compound_delete(key)
        else:
            raise TypeError("Tag type cannot be indexed")
    def __iter__(self):
        typ = self.get_type()
        if typ == TAG_LIST:
            it = c_void_p(0)
            tag = c_void_p(0)
            self._list_iterator_init(self, ctypes.byref(it))
            while self._list_iterator_next(ctypes.byref(it), ctypes.byref(tag)):
                if not tag.value:
                    return
                yield Tag(tag.value, True)
        elif typ == TAG_COMPOUND:
            for key, value in self.iteritems():
                yield key
        else:
            raise TypeError("Tag type cannot be iterated")
    
    # list specific conveniences
    def append(self, x):
        self.list_insert(self.list_get_length(), x)
    def extend(self, L):
        length = self.list_get_length()
        for i, x in enumerate(L):
            self.list_insert(length + i, x)
    def insert(self, i, x):
        if i < 0:
            i += self.list_get_length()
        self.list_insert(i, x)
    def reverse(self):
        self.list_reverse()
    def index(self, x):
        if not self.get_type() == TAG_LIST:
            raise TypeError("Tag is not a list")
        for i, otherx in enumerate(self):
            if otherx == x:
                return i
        raise ValueError("x not in list")
    def remove(self, x):
        i = self.index(x)
        del self[i]
    
    # compound-specific conveniences
    def iteritems(self):
        if not self.get_type() == TAG_COMPOUND:
            raise TypeError("Tag is not a compound object")
        it = c_void_p(0)
        key = c_void_p(0)
        value = c_void_p(0)
        self._compound_iterator_init(self, ctypes.byref(it))
        while self._compound_iterator_next(ctypes.byref(it), ctypes.byref(key), ctypes.byref(value)):
            if not key.value or not value.value:
                return
            yield (ctypes.string_at(key), Tag(value.value, True))
    def iterkeys(self):
        for key, val in self.iteritems():
            yield key
    def itervalues(self):
        for key, val in self.iteritems():
            yield val
    def keys(self):
        return list(self.iterkeys())
    def values(self):
        return list(self.itervalues())
    def items(self):
        return list(self.iteritems())
    def has_key(self, key):
        return key in self.keys()
    def get(self, key):
        return self[key]
    def clear(self):
        for key in self.keys():
            del self[key]
    
    # more generic conveniences
    def pop(self, i):
        val = self[i]
        del self[i]
        return val
    
    # generic value getter/setter
    def get_value(self):
        typ = self.get_type()
        if typ in [TAG_BYTE, TAG_SHORT, TAG_INT, TAG_LONG]:
            return self.get_integer()
        elif typ in [TAG_FLOAT, TAG_DOUBLE]:
            return self.get_float()
        elif typ == TAG_BYTE_ARRAY:
            return self.get_byte_array()
        elif typ == TAG_STRING:
            return self.get_string()
        elif typ == TAG_LIST:
            return list(self)
        elif typ == TAG_COMPOUND:
            return dict(self)
        else:
            raise RuntimeError("unhandled RS Tag type")
    def set_value(self, value):
        typ = self.get_type()
        if typ in [TAG_BYTE, TAG_SHORT, TAG_INT, TAG_LONG]:
            self.set_integer(value)
        elif typ in [TAG_FLOAT, TAG_DOUBLE]:
            self.set_float(value)
        elif typ == TAG_BYTE_ARRAY:
            self.set_byte_array(value)
        elif typ == TAG_STRING:
            self.set_string(value)
        elif typ == TAG_LIST:
            if not isinstance(value, list):
                raise TypeError("value not a list")
            if any([(not isinstance(v, Tag)) for v in value]):
                raise TypeError("not all elements of list are Tag objects")
            while len(self) > 0:
                del self[0]
            value.reverse()
            for v in value:
                v.insert(0, v)
        elif typ == TAG_COMPOUND:
            if not isinstance(value, dict):
                raise TypeError("value not a dict")
            if any([(not isinstance(k, str)) for k in value.iterkeys()]):
                raise TypeError("not all keys are strings")
            if any([(not isinstance(v, Tag)) for v in value.itervalues()]):
                raise TypeError("not all values are Tags")
            self.clear()
            for k, v in value.iteritems():
                self[k] = v
        else:
            raise RuntimeError("unhandled RS Tag type")
    value = property(get_value, set_value)

##
## region.h
##

class Region(RedstoneObject):
    class Methods:
        open = (c_void_p, [c_char_p, c_bool])
        close = (None, [c_void_p])
        get_chunk_timestamp = (c_uint32, [c_void_p, c_uint8, c_uint8])
        get_chunk_length = (c_uint32, [c_void_p, c_uint8, c_uint8])
        get_chunk_compression = (c_int, [c_void_p, c_uint8, c_uint8])
        get_chunk_data = (c_void_p, [c_void_p, c_uint8, c_uint8])
        set_chunk_data = (None, [c_void_p, c_uint8, c_uint8, c_void_p, c_uint32, c_int])
        set_chunk_data_full = (None, [c_void_p, c_uint8, c_uint8, c_void_p, c_uint32, c_int, c_uint32])
        clear_chunk = (None, [c_void_p, c_uint8, c_uint8])
        flush = (None, [c_void_p])
    
    _destructor_ = "_close"
    
    @classmethod
    def open(cls, path, write=False):
        ptr = cls._open(path, bool(write))
        if not ptr:
            raise RuntimeError("could not read region file: %s" % (path,))
        return cls(ptr)
    
    def get_chunk_timestamp(self, x, z):
        return self._get_chunk_timestamp(self, x, z)
    def get_chunk_length(self, x, z):
        return self._get_chunk_length(self, x, z)
    def get_chunk_compression(self, x, z):
        return self._get_chunk_compression(self, x, z)
    def get_chunk_data(self, x, z):
        l = self.get_chunk_length(x, z)
        ptr = self._get_chunk_data(self, x, z)
        return ctypes.string_at(ptr, l)
    def contains_chunk(self, x, z):
        if self.get_chunk_timestamp(x, z) == 0 or self.get_chunk_length(x, z) <= 0:
            return False
        return True
    
    def set_chunk_data(self, x, z, data, enc, timestamp=None):
        inbuf = ctypes.create_string_buffer(data)
        if timestamp:
            self._set_chunk_data_full(self, x, z, data, ctypes.sizeof(data), enc, timestamp)
        else:
            self._set_chunk_data(self, x, z, data, ctypes.sizeof(data), enc)
    
    def clear_chunk(self, x, z):
        self._clear_chunk(self, x, z)
    def flush(self):
        self._flush(self)

##
## nbt.h
##

class NBT(RedstoneObject):
    class Methods:
        new = (c_void_p, [])
        parse = (c_void_p, [c_void_p, c_size_t, c_uint])
        parse_from_region = (c_void_p, [c_void_p, c_uint8, c_uint8])
        parse_from_file = (c_void_p, [c_char_p])
        free = (None, [c_void_p])
        
        write = (c_bool, [c_void_p, c_void_p, c_void_p, c_uint])
        write_to_region = (c_bool, [c_void_p, c_void_p, c_uint8, c_uint8])
        write_to_file = (c_bool, [c_void_p, c_char_p])
    class Properties:
        name = (c_char_p, str, None, None)
        root = (c_void_p, Tag, lambda ptr: Tag(ptr, True) if ptr else None)
    
    @classmethod
    def new(cls):
        return cls(cls._new())
    
    @classmethod
    def parse(cls, data, enc=AUTO_COMPRESSION):
        inbuf = ctypes.create_string_buffer(data)
        ptr = cls._parse(inbuf, ctypes.sizeof(inbuf), enc)
        if not ptr:
            raise RuntimeError("could not parse NBT string")
        return cls(ptr)
    
    @classmethod
    def parse_from_region(cls, region, x, z):
        if not isinstance(region, Region):
            raise TypeError("given region is not a Region")
        ptr = cls._parse_from_region(region, x, z)
        if not ptr:
            raise RuntimeError("could not parse NBT from region")
        return cls(ptr)
    
    @classmethod
    def parse_from_file(cls, fname):
        ptr = cls._parse_from_file(fname)
        if not ptr:
            raise RuntimeError("could not read NBT file: %s" % (fname,))
        return cls(ptr)
    
    def write(self, enc):
        outptr = c_void_p(0)
        outlen = c_size_t(0)
        success = self._write(self, ctypes.byref(outptr), ctypes.byref(outlen), enc)
        if not success:
            raise RuntimeError("could not write NBT")
        out = ctypes.string_at(outptr, outlen.value)
        rs.rs_free(outptr)
        
        return out
    
    def write_to_region(self, region, x, z):
        if not isinstance(region, Region):
            raise TypeError("given region is not a Region")
        success = self._write_to_region(self, region, x, z)
        if not success:
            raise RuntimeError("could not write NBT to region")
    
    def write_to_file(self, out):
        success = self._write_to_file(self, out)
        if not success:
            raise RuntimeError("could not write NBT to file: %s" % (out,))
    
    # convenience macros
    def find(self, name):
        return self.root.find(name)
    def print_(self, dest=sys.stdout):
        return self.root.print_(dest)
    def pretty_print(self, dest=sys.stdout):
        return self.root.pretty_print(dest)
    
    # conveniences specific to python
    def __len__(self):
        return len(self.root)
    def __iter__(self):
        return iter(self.root)
    def __getitem__(self, k):
        return self.root[k]
    def __setitem__(self, k, val):
        self.root[k] = val
