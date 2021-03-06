/*
Pack/UnPack Library
Originally developed at the UIUC Parallel Programming Lab
by Orion Sky Lawlor, olawlor@acm.org, 4/5/2000

This library allows you to easily pack an array, structure,
or object into a memory buffer or disk file, and then read
the object back later.  The library can also handle translating
between different machine representations for integers and floats.

Originally, you had to write separate functions for buffer 
sizing, pack to memory, unpack from memory, pack to disk, and 
unpack from disk.  These functions all perform the exact same function--
namely, they list the members of the array, struct, or object.
Further, all the functions must agree, or the unpacked data will 
be garbage.  This library allows the user to write *one* function,
pup, which will perform all needed packing/unpacking.

A simple example is:
class foo {
 private:
  bool isBar;
  int x;
  char y;
  unsigned long z;
  PupVec<double> q;
 public:
  ...
  void pup(PUP::er &p) {
    PUPn(isBar);
    PUPn(x);PUPn(y);PUPn(z);
    PUPn(q);
  }
};

A more complex example is:
class bar {
 private:
  foo f;
  int nArr;//Length of array below
  double *arr;//Heap-allocated array
 public:
  ...
  
  void pup(PUP::er &p) {
    PUPn(f); // <- automatically calls foo::pup
    PUPn(nArr);
    if (p.isUnpacking()) // <- must allocate array on other side.
      arr=new double[nArr];
    PUPv(arr,nArr); // <- special syntax for arrays of simple types
  }
};
*/

#ifndef __LAWLOR_PUP_H
#define __LAWLOR_PUP_H
#define __CK_PUP_H 1 /* backward compatability */
#include "pup/pup_dll.h" /* grap PUP_DLL macro */

#include <stdio.h> /*<- for "FILE *" */

#ifndef __cplusplus
#error "Use pup_c.h for C programs-- pup.h is for C++ programs"
#endif

#include "pup_config.h"

//We need PupMigrateMessage only to distinguish the migration
// constructor from all other constructors-- the type
// itself has no meaningful fields.
typedef struct {int is_only_a_name;} PupMigrateMessage;

// This is a generic abort routine-- print an error message and exit.
extern "C" void CmiAbort(const char *msg);

namespace PUP {
 
//Item data types-- these are used to do byte swapping, etc.
typedef enum {
//(this list must exactly match that in PUPer_xlate)
  Tchar=0,Tshort, Tint, Tlong, Tlonglong,
  Tuchar,Tushort,Tuint,Tulong, Tulonglong,
  Tfloat,Tdouble,Tlongdouble,
  Tbool,
  Tbyte,
  Tsync,
  dataType_last //<- for setting table lengths, etc.
} dataType;

//This should be a 1-byte unsigned type
typedef unsigned char myByte;

//Forward declarations
class er;
class able;

//Used for out-of-order unpacking
class PUP_DLL seekBlock {
	enum {maxSections=3};
	int secTab[maxSections+1];//The start of each seek section
	int nSec;//Number of sections; current section #
	int secTabOff;//Start of the section table, relative to the seek block
	er &p;
	bool hasEnded;
public:
	//Constructor
	seekBlock(er &Np,int nSections);
	//Destructor
	~seekBlock();

	//Seek to the given section number (0-based, less than nSections)
	void seek(int toSection);
	//Finish with this seeker (must be called)
	void endBlock(void);

	//An evil hack to avoid inheritance and virtual functions among seekers--
	// stores the PUP::er specific block start information.
	union {
		int off;
		long loff;
		const myByte *cptr;
		myByte *ptr;
		void *vptr;
	} data;
};

//The abstract base class:  PUP::er.
class PUP_DLL er {
 private:
  er(const er &p);//You don't want to copy PUP::er's.
 protected:
  /// These state bits describe various user-settable properties.
  typedef enum {IS_USERLEVEL=0x0004, // If set, this is *not* a migration pup-- it's something else.
	IS_DELETING =0x0008, // If set, C & f90 objects should delete themselves after pup
	IS_COMMENTS =0x0010  // If set, this PUP::er wants comments and sync codes.
  } PUP_er_properties;
  /// These state bits describe the PUP::er's direction.
  typedef enum {IS_SIZING   =0x0100,
  	IS_PACKING  =0x0200,
        IS_UNPACKING=0x0400,
        TYPE_MASK   =0xFF00
  } PUP_er_direction;
  /// This is a bitwise OR of PUP_er_properties and PUP_er_direction.
  unsigned int PUP_er_state;
  er(PUP_er_direction inType) //You don't want to create raw PUP::er's.
              {PUP_er_state=(unsigned int)inType;}
 public:
  virtual ~er();//<- does nothing, but might be needed by some child

  //State queries (exactly one of these will be true)
  bool isSizing(void) const {return (PUP_er_state&IS_SIZING)!=0?true:false;}
  bool isPacking(void) const {return (PUP_er_state&IS_PACKING)!=0?true:false;}
  bool isUnpacking(void) const {return (PUP_er_state&IS_UNPACKING)!=0?true:false;}
  char *  typeString() const;
  unsigned int getStateFlags(void) const {return PUP_er_state;}

  //This indicates that the pup routine should free memory during packing.
  void becomeDeleting(void) {PUP_er_state|=IS_DELETING;}
  bool isDeleting(void) const {return (PUP_er_state&IS_DELETING)!=0?true:false;}

  //This indicates that the pup routine should not call system objects' pups.
  void becomeUserlevel(void) {PUP_er_state|=IS_USERLEVEL;}
  bool isUserlevel(void) const {return (PUP_er_state&IS_USERLEVEL)!=0?true:false;}
  
  bool hasComments(void) const {return (PUP_er_state&IS_COMMENTS)!=0?true:false;}

//For single elements, pretend it's an array containing one element
  void operator()(signed char &v)     {(*this)(&v,1);}
#if PUP_SIGNEDCHAR_DIFF_CHAR
  void operator()(char &v)            {(*this)(&v,1);}
#endif
  void operator()(short &v)           {(*this)(&v,1);}
  void operator()(int &v)             {(*this)(&v,1);}
  void operator()(long &v)            {(*this)(&v,1);}
  void operator()(unsigned char &v)   {(*this)(&v,1);}
  void operator()(unsigned short &v)  {(*this)(&v,1);}
  void operator()(unsigned int &v)    {(*this)(&v,1);}
  void operator()(unsigned long &v)   {(*this)(&v,1);}
  void operator()(float &v)           {(*this)(&v,1);}
  void operator()(double &v)          {(*this)(&v,1);}
#if PUP_LONG_DOUBLE_DEFINED
  void operator()(long double &v)     {(*this)(&v,1);}
#endif
  void operator()(bool &v)         {(*this)(&v,1);}
#ifdef PUP_LONG_LONG
  void operator()(PUP_LONG_LONG &v) {(*this)(&v,1);}
  void operator()(unsigned PUP_LONG_LONG &v) {(*this)(&v,1);}
#endif

//For arrays:
  //Integral types:
  void operator()(signed char *a,int nItems)
    {bytes((void *)a,nItems,sizeof(signed char),Tchar);}
#if PUP_SIGNEDCHAR_DIFF_CHAR
  void operator()(char *a,int nItems)
    {bytes((void *)a,nItems,sizeof(char),Tchar);}
#endif
  void operator()(short *a,int nItems)
    {bytes((void *)a,nItems,sizeof(short),Tshort);}
  void operator()(int *a,int nItems)
    {bytes((void *)a,nItems,sizeof(int),Tint);}
  void operator()(long *a,int nItems)
    {bytes((void *)a,nItems,sizeof(long),Tlong);}

  //Unsigned integral types:
  void operator()(unsigned char *a,int nItems)
    {bytes((void *)a,nItems,sizeof(unsigned char),Tuchar);}
  void operator()(unsigned short *a,int nItems)
    {bytes((void *)a,nItems,sizeof(unsigned short),Tushort);}
  void operator()(unsigned int *a,int nItems)
    {bytes((void *)a,nItems,sizeof(unsigned int),Tuint);}
  void operator()(unsigned long *a,int nItems)
    {bytes((void *)a,nItems,sizeof(unsigned long),Tulong);}

  //Floating-point types:
  void operator()(float *a,int nItems)
    {bytes((void *)a,nItems,sizeof(float),Tfloat);}
  void operator()(double *a,int nItems)
    {bytes((void *)a,nItems,sizeof(double),Tdouble);}

#if PUP_LONG_DOUBLE_DEFINED
  void operator()(long double *a,int nItems)
    {bytes((void *)a,nItems,sizeof(long double),Tlongdouble);}
#endif

  //For bools:
  void operator()(bool *a,int nItems)
    {bytes((void *)a,nItems,sizeof(bool),Tbool);}

#ifdef PUP_LONG_LONG
  void operator()(PUP_LONG_LONG *a,int nItems)
    {bytes((void *)a,nItems,sizeof(PUP_LONG_LONG),Tlonglong);}
  void operator()(unsigned PUP_LONG_LONG *a,int nItems)
    {bytes((void *)a,nItems,sizeof(unsigned PUP_LONG_LONG),Tulonglong);}
#endif

  //For raw memory (n gives number of bytes)
  void operator()(void *a,int nBytes)
    {bytes((void *)a,nBytes,1,Tbyte);}

  //For allocatable objects (system will new/delete object and call pup routine)
  void operator()(able** a)
    {object(a);}
  //For pre- or stack-allocated PUP::able objects-- just call object's pup
  void operator()(able& a);

  /// A descriptive (but entirely optional) human-readable comment field
  virtual void comment(const char *message);

  /// A 32-bit "synchronization marker" (not human readable).
  ///  Some standard codes are listed under PUP::sync_....
  virtual void synchronize(unsigned int sync);
  
  /// Insert a synchronization marker and comment into the stream.
  ///  Only applies if this PUP::er wants comments.
#ifndef PUP_OPTIMIZE
  inline void syncComment(unsigned int sync,const char *message=0) {
  	if (hasComments()) {
		synchronize(sync);
		if (message) comment(message);
	}
  }
#else
  inline void syncComment(unsigned int sync,const char *message=0) {
  	/* empty, to avoid expensive virtual function calls */
  }
#endif

  //Generic bottleneck: pack/unpack n items of size itemSize
  // and data type t from p.  Desc describes the data item
  virtual void bytes(void *p,int n,size_t itemSize,dataType t) =0;
  virtual void object(able** a);

  //For seeking (pack/unpack in different orders)
  virtual void impl_startSeek(seekBlock &s); /*Begin a seeking block*/
  virtual int impl_tell(seekBlock &s); /*Give the current offset*/
  virtual void impl_seek(seekBlock &s,int off); /*Seek to the given offset*/
  virtual void impl_endSeek(seekBlock &s);/*End a seeking block*/
};

/**
 "Sync" codes are an extra channel to encode data in a pup stream, 
 to indicate higher-order structures in the pup'd objects.
 Sync codes must follow this grammar:
   <obj> -> <obj> <obj> | <array> | <list>
   <obj> -> begin (<obj> system) <obj> end
   <array> -> begin <obj> (item <obj>)* end
   <list> -> begin <obj> (index <obj> item <obj>)* end
 This hack is used, e.g., by the debugger.
*/
enum {
  sync_builtin=0x70000000, // Built-in, standard sync codes begin here
  sync_begin=sync_builtin+0x01000000, // Sync code at start of collection
  sync_end=sync_builtin+0x02000000, // Sync code at end of collection
  sync_last_system=sync_builtin+0x09000000, // Sync code at end of "system" portion of object
  sync_array_m=0x00100000, // Linear-indexed (0..n) array-- use item to separate
  sync_list_m=0x00200000, // Some other collection-- use index and item
  sync_object_m=0x00300000, // Sync mask for general object
  
  sync_begin_array=sync_begin+sync_array_m,
  sync_begin_list=sync_begin+sync_list_m, 
  sync_begin_object=sync_begin+sync_object_m, 
  
  sync_end_array=sync_end+sync_array_m, 
  sync_end_list=sync_end+sync_list_m, 
  sync_end_object=sync_end+sync_object_m, 
  
  sync_item=sync_builtin+0x00110000, // Sync code for a list or array item
  sync_index=sync_builtin+0x00120000, // Sync code for index of item in a list
  
  sync_last
};

/************** PUP::er -- Sizer ******************/
//For finding the number of bytes to pack (e.g., to preallocate a memory buffer)
class PUP_DLL sizer : public er {
 protected:
  int nBytes;
  //Generic bottleneck: n items of size itemSize
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
 public:
  //Write data to the given buffer
  sizer(void):er(IS_SIZING) {nBytes=0;}
  
  //Return the current number of bytes to be packed
  int size(void) const {return nBytes;}
};

template <class T>
inline int size(T &t) {
	PUP::sizer p; PUPt(p,t); return p.size();
}

/********** PUP::er -- Binary memory buffer pack/unpack *********/
class PUP_DLL mem : public er { //Memory-buffer packers and unpackers
 protected:
  myByte *origBuf;//Start of memory buffer
  myByte *buf;//Memory buffer (stuff gets packed into/out of here)
  mem(PUP_er_direction type,myByte *Nbuf):er(type),origBuf(Nbuf),buf(Nbuf) {}

  //For seeking (pack/unpack in different orders)
  virtual void impl_startSeek(seekBlock &s); /*Begin a seeking block*/
  virtual int impl_tell(seekBlock &s); /*Give the current offset*/
  virtual void impl_seek(seekBlock &s,int off); /*Seek to the given offset*/
 public:
  //Return the current number of buffer bytes used
  int size(void) const {return buf-origBuf;}
};

//For packing into a preallocated, presized memory buffer
class PUP_DLL toMem : public mem {
 protected:
  //Generic bottleneck: pack n items of size itemSize from p.
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
 public:
  //Write data to the given buffer
  toMem(void *Nbuf):mem(IS_PACKING,(myByte *)Nbuf) {}
};
template <class T>
inline void toMemBuf(T &t,void *buf,int len) {
	PUP::toMem p(buf);
	PUPt(p,t);
	if (p.size()!=len) CmiAbort("Size mismatch during PUP::toMemBuf!\n"
		"This means your pup routine doesn't match during sizing and packing");
}

//For unpacking from a memory buffer
class PUP_DLL fromMem : public mem {
 protected:
  //Generic bottleneck: unpack n items of size itemSize from p.
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
 public:
  //Read data from the given buffer
  fromMem(const void *Nbuf):mem(IS_UNPACKING,(myByte *)Nbuf) {}
};
template <class T>
inline void fromMemBuf(T &t,void *buf,int len) {
	PUP::fromMem p(buf);
	PUPt(p,t);
	if (p.size()!=len) CmiAbort("Size mismatch during PUP::fromMemBuf!\n"
		"This means your pup routine doesn't match during packing and unpacking");
}

/********** PUP::er -- Binary disk file pack/unpack *********/
class PUP_DLL disk : public er {
 protected:
  FILE *F;//Disk file to read from/write to
  disk(PUP_er_direction type,FILE *f):er(type),F(f) {}

  //For seeking (pack/unpack in different orders)
  virtual void impl_startSeek(seekBlock &s); /*Begin a seeking block*/
  virtual int impl_tell(seekBlock &s); /*Give the current offset*/
  virtual void impl_seek(seekBlock &s,int off); /*Seek to the given offset*/
};

//For packing to a disk file
class PUP_DLL toDisk : public disk {
 protected:
  //Generic bottleneck: pack n items of size itemSize from p.
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
 public:
  //Write data to the given file pointer
  // (must be opened for binary write)
  // You must close the file yourself when done.
  toDisk(FILE *f):disk(IS_PACKING,f) {}
};

//For unpacking from a disk file
class PUP_DLL fromDisk : public disk {
 protected:
  //Generic bottleneck: unpack n items of size itemSize from p.
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
 public:
  //Write data to the given file pointer 
  // (must be opened for binary read)
  // You must close the file yourself when done.
  fromDisk(FILE *f):disk(IS_UNPACKING,f) {}
};



/************** PUP::er -- Text *****************/
class PUP_DLL toTextUtil : public er {
 private:
  char *cur; /*Current output buffer*/
  int level; /*Indentation distance*/
  void beginEnv(const char *type,int n=0);
  void endEnv(const char *type);
  char *beginLine(void);
  void endLine(void);
 protected:
  virtual char *advance(char *cur)=0; /*Consume current buffer and return next*/
  toTextUtil(PUP_er_direction inType,char *buf);
 public:
  virtual void comment(const char *message);
  virtual void synchronize(unsigned int m);
 protected:
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
  virtual void object(able** a);
};
/* Return the number of characters, including terminating NULL */
class PUP_DLL sizerText : public toTextUtil {
 private:
  char line[1000];
  int charCount; /*Total characters seen so far (not including NULL) */
 protected:
  virtual char *advance(char *cur);
 public:
  sizerText(void);
  int size(void) const {return charCount+1; /*add NULL*/ }
};
/* Copy data to this C string, including terminating NULL. */
class PUP_DLL toText : public toTextUtil {
 private:
  char *buf;
  int charCount; /*Total characters written so far (not including NULL) */
 protected:
  virtual char *advance(char *cur);
 public:
  toText(char *outStr);
  int size(void) const {return charCount+1; /*add NULL*/ }
};

class PUP_DLL toTextFile : public er {
 protected:
  FILE *f;
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
 public:
  //Begin writing to this file, which should be opened for ascii write.
  // You must close the file yourself when done.
  toTextFile(FILE *f_,bool withComments=false) 
  	:er(withComments?PUP_er_direction(IS_PACKING+IS_COMMENTS):IS_PACKING), f(f_) 
	{}
  virtual void comment(const char *message);
};
class PUP_DLL fromTextFile : public er {
 protected:
  FILE *f;
  int readInt(const char *fmt="%d");
  unsigned int readUint(const char *fmt="%u");
#ifdef PUP_LONG_LONG
  PUP_LONG_LONG readLongInt(const char *fmt="%lld");
#endif
  double readDouble(void);
  
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
  virtual void parseError(const char *what);
 public:
  //Begin writing to this file, which should be opened for ascii read.
  // You must close the file yourself when done.
  fromTextFile(FILE *f_) :er(IS_UNPACKING), f(f_) {}
  virtual void comment(const char *message);
};

/********** PUP::er -- Heterogenous machine pack/unpack *********/
//This object describes the data representation of a machine.
class PUP_DLL machineInfo {
 public:
  typedef unsigned char myByte;
  myByte magic[4];//Magic number (to identify machineInfo structs)
  myByte version;//0-- current version

  myByte intBytes[4]; //<- sizeof(char,short,int,long)
  myByte intFormat;//0-- big endian.  1-- little endian.

  myByte floatBytes; //<- sizeof(...)
  myByte doubleBytes;
  myByte floatFormat;//0-- big endian IEEE.  1-- little endian IEEE.

  myByte boolBytes;

  myByte padding[2];//Padding to 16 bytes

  //Return true if our magic number is valid.
  bool valid(void) const;
  //Return true if we differ from the current (running) machine.
  bool needsConversion(void) const;
  
  //Get a machineInfo for the current machine
  static const machineInfo &current(void);
};

/// "Wrapped" PUP::er: forwards requests to another PUP::er.
class PUP_DLL wrap_er : public er {
protected:
	er &p;
public:
	wrap_er(er &p_,unsigned int newFlags=0) :er(PUP_er_direction(p_.getStateFlags()|newFlags)), p(p_) {}
	
	virtual void impl_startSeek(seekBlock &s); /*Begin a seeking block*/
	virtual int impl_tell(seekBlock &s); /*Give the current offset*/
	virtual void impl_seek(seekBlock &s,int off); /*Seek to the given offset*/
	virtual void impl_endSeek(seekBlock &s);/*End a seeking block*/
};

//For translating some odd disk/memory representation into the 
// current machine representation.  (We really only need to
// translate during unpack-- "reader makes right".)
class PUP_DLL xlater : public wrap_er {
 protected:
  typedef void (*dataConverterFn)(int N,const myByte *in,myByte *out,int nElem);
  
  //This table is indexed by dataType, and contains an appropriate
  // conversion function to unpack a n-item array of the corresponding 
  // data type (possibly in-place).
  dataConverterFn convertFn[dataType_last];
  //Maps dataType to source machine's dataSize
  size_t convertSize[dataType_last];
  void setConverterInt(const machineInfo &m,const machineInfo &cur,
    int isUnsigned,int intType,dataType dest);
  
  //Generic bottleneck: unpack n items of size itemSize from p.
  virtual void bytes(void *p,int n,size_t itemSize,dataType t);
 public:
  xlater(const machineInfo &fromMachine, er &fromData);
};

/*************** PUP::able support ***************/
//The base class of system-allocatable objects with pup routines
class PUP_DLL able {
public:
	//A globally-unique, persistent identifier for a PUP::able object
	class PUP_ID {
	public:
		enum {len=8};
		unsigned char hash[len];
		PUP_ID() {}
		explicit PUP_ID(int val) {for (int i=0;i<len;i++) hash[i]=(unsigned char)val;}
		explicit PUP_ID(const char *name) {setName(name);}
		void setName(const char *name);//Write name into hash
		bool operator==(const PUP_ID &other) const {
			for (int i=0;i<len;i++)
				if (hash[i]!=other.hash[i])
					return false;
			return true;
		}
		bool operator!=(const PUP_ID &other) const {
			return !((*this)==other);
		}
		bool operator<(const PUP_ID &other) const {
			for (int i=0;i<len;i++) {
				if (hash[i]<other.hash[i])
					return true; /* really less */
				else if (hash[i]>other.hash[i])
					return false; /* really greater */
				/* else hash[i]==other.hash[i], so keep looking */
			}
			return false; /* equal */
		}
		void pup(er &p) {
			 p((void *)hash,sizeof(unsigned char)*len);
		}
		void pup(er &p) const {
			 p((void *)hash,sizeof(unsigned char)*len);
		}
	};

protected:
	able() {}
	able(PupMigrateMessage *) {}
	virtual ~able();//Virtual destructor may be needed by some child

public:
//Constructor function registration:
	typedef able* (*constructor_function)(void);
	static PUP_ID register_constructor(const char *className,
		constructor_function fn);
	static constructor_function get_constructor(const PUP_ID &id);
	virtual /*PUP::*/able *clone(void) const;

//Target methods:
	virtual void pup(er &p);
	virtual const PUP_ID &get_PUP_ID(void) const=0;
};

//Declarations to include in a PUP::able's body.
//  Convenient, but only usable if class is not inside a namespace.
#define PUPable_decl(className) \
    PUPable_decl_inside(className) \
    friend inline void PUPt(PUP::er &p,className &a) {a.pup(p);}\
    friend inline void PUPt(PUP::er &p,className* &a) {\
	PUP::able *pa=a;  p(&pa);  a=(className *)pa;\
    }

//PUPable_decl for classes inside a namespace: inside body
#define PUPable_decl_inside(className) \
private: \
    static PUP::able *call_PUP_constructor(void); \
    static PUP::able::PUP_ID my_PUP_ID;\
public:\
    virtual const PUP::able::PUP_ID &get_PUP_ID(void) const; \
    static void register_PUP_ID(void);

//PUPable_decl for classes inside a namespace: in header at file scope
#define PUPable_decl_outside(className) \
    inline void PUPt(PUP::er &p,className &a) {a.pup(p);}\
    inline void PUPt(PUP::er &p,className* &a) {\
	PUP::able *pa=a;  p(&pa);  a=(className *)pa;\
    }


//Declarations to include in an abstract PUP::able's body.
//  Abstract PUP::ables do not need def or reg.
#define PUPable_abstract(className) \
public:\
    virtual const PUP::able::PUP_ID &get_PUP_ID(void) const =0; \
    friend inline void PUPt(PUP::er &p,className &a) {a.pup(p);}\
    friend inline void PUPt(PUP::er &p,className* &a) {\
	PUP::able *pa=a;  p(&pa);  a=(className *)pa;\
    }

//Definitions to include exactly once at file scope
#define PUPable_def(className) \
	PUP::able *className::call_PUP_constructor(void) \
		{ return new className((PupMigrateMessage *)0);}\
	const PUP::able::PUP_ID &className::get_PUP_ID(void) const\
		{ return className::my_PUP_ID; }\
	PUP::able::PUP_ID className::my_PUP_ID;\
	void className::register_PUP_ID(void)\
		{my_PUP_ID=register_constructor(#className,\
		              className::call_PUP_constructor);}\

//Code to execute exactly once at program start time
#define PUPable_reg(className) \
    className::register_PUP_ID();


}//<- End namespace PUP

inline void PUPt(PUP::er &p,PUP::able &a) {a.pup(p);}
inline void PUPt(PUP::er &p,PUP::able* &a) {p(&a);}

//Holds a pointer to a (possibly dynamically allocated) PUP::able.
//  Extracting the pointer hands the deletion responsibility over.
//  This is used by parameter marshalling, which doesn't work well 
//  with bare pointers.
//   PupPointer<T> t   is the parameter-marshalling equivalent of   T *t
template <class T>
class PupPointer {
	T *allocated; //Pointer that PUP dynamically allocated for us (recv only)
	T *ptr; //Read-only pointer

#if 0 /* Private (do-not-use) copy constructor.  This prevents allocated from being
         deleted twice--once in the original, and again in the copy.*/
	PupPointer(const PupPointer<T> &src); // Don't use this!
#else /* Some compilers, like gcc3, have a hideous bug that causes them to *demand*
         a public copy constructor when a class is used to initialize a const-reference
	 from a temporary.  The public copy constructor should never be called, though. */
public:
	PupPointer(const PupPointer<T> &src) {
		CmiAbort("PUPable_marshall's cannot be passed by value.  Pass them only by reference!");
	}
#endif
protected:
	T *peek(void) {return ptr;}
public:
	/// Used on the send side, and does *not* delete the object.
	PupPointer(T *src)  ///< Marshall this object.
	{ 
		allocated=0; //Don't ever delete src
		ptr=src;
	}
	
	/// Begin completely empty: used on marshalling recv side.
	PupPointer(void) { 
		ptr=allocated=0;
	}
	
	~PupPointer() { if (allocated) delete allocated; }
	
	/// Extract the object held by this class.  
	///  Deleting the pointer is now the user's responsibility
	inline operator T* () { allocated=0; return ptr; }
	
	inline void pup(PUP::er &p) {
		bool ptrWasNull=(ptr==0);
		
		PUP::able *ptr_able=ptr; // T must inherit from PUP::able!
		PUPt(p,ptr_able); //Pack as a PUP::able *
		ptr=(T *)ptr_able;
		
		if (ptrWasNull) 
		{ //PUP just allocated a new object for us-- 
		  // make sure it gets deleted eventually.
			allocated=ptr;
		}
	}
	friend inline void PUPt(PUP::er &p,PupPointer<T> &v) {v.pup(p);}
};
#define PUPable_marshall PupPointer

//Like PupPointer, but keeps deletion responsibility forever.
//   PupReference<T> t  is the parameter-marshalling equivalent of   T &t
template<class T>
class PupReference : private PupPointer<T> {
public:
	/// Used on the send side, and does *not* delete the object.
	PupReference(T &src)   ///< Marshall this object.
		:PupPointer<T>(&src) { }
	
	/// Begin completely empty: used on the recv side.
	PupReference(void) {}
	
	/// Look at the object held by this class.  Does *not* hand over
	/// deletion responsiblity.
	inline operator T& () { return *this->peek(); }
	
	inline void pup(PUP::er &p) {PupPointer<T>::pup(p);}
	
	friend inline void PUPt(PUP::er &p,PupReference<T> &v) {v.pup(p);}
};

// For people that forget the "::"
typedef PUP::er PUPer;
typedef PUP::able PUPable;

/******** PUP via pipe: another way to access PUP::ers *****
The parameter marshalling system pups each variable v using just:
     PUPt(p,v);
Thus we need a "void PUPt(PUP::er &p,T &v)" for all types
that work with parameter marshalling. 
*/

/// Normal case: Call pup routines by default
/**
  Default PUPt: call pup routine.
*/
template<class T>
inline void PUPt(PUP::er &p,T &t) { 
	p.syncComment(PUP::sync_begin_object);
	t.pup(p);
	p.syncComment(PUP::sync_end_object); 
}

/**
  Default PUParray: pup each element.
*/
template<class T>
inline void PUParray(PUP::er &p,T *t,int n) {
	p.syncComment(PUP::sync_begin_array);
	for (int i=0;i<n;i++) {
		p.syncComment(PUP::sync_item);
		PUPt(p,t[i]);
	}
	p.syncComment(PUP::sync_end_array);
}

/// Copy this type as raw memory (like memcpy).
#define PUPbytes(type) \
  inline void PUPt(PUP::er &p,type &t) {p((void *)&t,sizeof(type));} \
  inline void PUParray(PUP::er &p,type *ta,int n) { p((void *)ta,n*sizeof(type)); } \
  namespace PUP { template<> class as_bytes<type> { \
  	public: enum {value=1};  \
  }; };
#define PUPmarshallBytes(type) PUPbytes(type)

/// Make PUP work with this function pointer type, copied as raw bytes.
#define PUPfunctionpointer(fnPtrType) \
  inline void PUPt(PUP::er &p,fnPtrType &t) {p((void *)&t,sizeof(fnPtrType));}

/// Make PUP work with this enum type, copied as an "int".
#define PUPenum(enumType) \
  inline void PUPt(PUP::er &p,enumType &e) { int v=e;  PUPt(p,v); e=v; }


/**
  For all builtin types, like "int",
  PUPt and PUParray use p(t) and p(ta,n).
*/
#define PUP_BUILTIN_SUPPORT(type) \
  inline void PUPt(PUP::er &p,type &t) {p(t);} \
  inline void PUParray(PUP::er &p,type *ta,int n) { p(ta,n); } 
PUP_BUILTIN_SUPPORT(signed char)
#if PUP_SIGNEDCHAR_DIFF_CHAR
PUP_BUILTIN_SUPPORT(char)
#endif
PUP_BUILTIN_SUPPORT(unsigned char)
PUP_BUILTIN_SUPPORT(short)
PUP_BUILTIN_SUPPORT(int)
PUP_BUILTIN_SUPPORT(long)
PUP_BUILTIN_SUPPORT(unsigned short)
PUP_BUILTIN_SUPPORT(unsigned int)
PUP_BUILTIN_SUPPORT(unsigned long)
PUP_BUILTIN_SUPPORT(float)
PUP_BUILTIN_SUPPORT(double)
PUP_BUILTIN_SUPPORT(bool)
#if PUP_LONG_DOUBLE_DEFINED
PUP_BUILTIN_SUPPORT(long double)
#endif
#ifdef PUP_LONG_LONG
PUP_BUILTIN_SUPPORT(PUP_LONG_LONG)
PUP_BUILTIN_SUPPORT(unsigned PUP_LONG_LONG)
#endif
#undef PUP_BUILTIN_SUPPORT

//This macro is useful in simple pup routines:
//  It's just PUPt(p,x, but it also documents the *name* of the variable.)
// You must name your PUP::er "p".
#define PUPn(field) \
  do{  if (p.hasComments()) p.comment(#field); PUPt(p,field); } while(0)

//Like PUPn(x), above, but for arrays.
#define PUPv(field,len) \
  do{  if (p.hasComments()) p.comment(#field); PUParray(p,field,len); } while(0)


#endif //def __LAWLOR_PUP_H


