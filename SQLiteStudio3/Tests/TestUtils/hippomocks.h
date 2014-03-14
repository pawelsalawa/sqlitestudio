/*
 * This is a Hippo Mocks, a mocking framework for C++.
 * It's created by a HippoMocks team, see https://www.assembla.com/spaces/hippomocks/team
 *
 * Project page: https://www.assembla.com/spaces/hippomocks/wiki
 *
 * Licensed under LGPL.
 */

#ifndef HIPPOMOCKS_H
#define HIPPOMOCKS_H

#ifndef EXCEPTION_BUFFER_SIZE
#define EXCEPTION_BUFFER_SIZE 65536
#endif

#ifndef VIRT_FUNC_LIMIT 
#define VIRT_FUNC_LIMIT 1024
#elif VIRT_FUNC_LIMIT > 1024
#error Adjust the code to support more than 1024 virtual functions before setting the VIRT_FUNC_LIMIT above 1024
#endif

#ifdef __EDG__
#define FUNCTION_BASE 3
#define FUNCTION_STRIDE 2
#else
#define FUNCTION_BASE 0
#define FUNCTION_STRIDE 1
#endif

#include <list>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include <cstring>

#ifdef _MSC_VER
// these warnings are pointless and huge, and will confuse new users. 
#pragma warning(push)
// If you can't generate an assignment operator the least you can do is shut up.
#pragma warning(disable: 4512)
// Alignment not right in a union?
#pragma warning(disable: 4121)
// Template parameters have the tendency not to change between executions.
#pragma warning(disable: 4127)
// No deprecated warnings on functions that really aren't deprecated at all.
#pragma warning(disable: 4996)
#endif
class MockRepository;

enum RegistrationType {
  Never,
  Once,
  DontCare
};

// base type
class base_mock {
public:
	void destroy() { unwriteVft(); delete this; }
	virtual ~base_mock() {}
	void *rewriteVft(void *newVf) 
	{
		void *oldVf = *(void **)this;
		*(void **)this = newVf;
		return oldVf;
	}
	void unwriteVft() 
	{
		*(void **)this = (*(void ***)this)[-1];
	}
};

class NullType 
{
public: 
	bool operator==(const NullType &) const 
	{
		return true; 
	}
};

struct NotPrintable { template <typename T> NotPrintable(T const&) {} };

inline std::ostream &operator<<(std::ostream &os, NotPrintable const&)
{
	os << "???";
	return os;
}

template <typename T>
struct printArg 
{
  static inline void print(std::ostream &os, T arg, bool withComma)
  {
    if (withComma) os << ",";
    os << arg;
  }
};

template <>
struct printArg<NullType>
{
  static void print(std::ostream &, NullType , bool)
  {
  }
};

class base_tuple 
{
protected:
	base_tuple() 
	{
	} 
public: 
	virtual ~base_tuple() 
	{
	}
	virtual bool operator==(const base_tuple &) const = 0;
  virtual void printTo(std::ostream &os) const = 0;
};

template <typename X>
struct no_cref { typedef X type; };

template <typename X>
struct no_cref<const X &> { typedef X type; };

struct NotComparable { template <typename T> NotComparable(const T&) {} };

inline bool operator==(NotComparable, NotComparable)
{
	return false;
}

template <typename T>
struct comparer
{
  static inline bool compare(T a, T b)
  {
    return a == b;
  }
};

template <typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType, 
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType, 
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType, 
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class tuple : public base_tuple
{
public:
	typename no_cref<A>::type a;
	typename no_cref<B>::type b;
	typename no_cref<C>::type c;
	typename no_cref<D>::type d;
	typename no_cref<E>::type e;
	typename no_cref<F>::type f;
	typename no_cref<G>::type g;
	typename no_cref<H>::type h;
	typename no_cref<I>::type i;
	typename no_cref<J>::type j;
	typename no_cref<K>::type k;
	typename no_cref<L>::type l;
	typename no_cref<M>::type m;
	typename no_cref<N>::type n;
	typename no_cref<O>::type o;
	typename no_cref<P>::type p;
	tuple(typename no_cref<A>::type a = typename no_cref<A>::type(), typename no_cref<B>::type b = typename no_cref<B>::type(), 
		  typename no_cref<C>::type c = typename no_cref<C>::type(), typename no_cref<D>::type d = typename no_cref<D>::type(), 
		  typename no_cref<E>::type e = typename no_cref<E>::type(), typename no_cref<F>::type f = typename no_cref<F>::type(), 
		  typename no_cref<G>::type g = typename no_cref<G>::type(), typename no_cref<H>::type h = typename no_cref<H>::type(),
		  typename no_cref<I>::type i = typename no_cref<I>::type(), typename no_cref<J>::type j = typename no_cref<J>::type(), 
		  typename no_cref<K>::type k = typename no_cref<K>::type(), typename no_cref<L>::type l = typename no_cref<L>::type(), 
		  typename no_cref<M>::type m = typename no_cref<M>::type(), typename no_cref<N>::type n = typename no_cref<N>::type(), 
		  typename no_cref<O>::type o = typename no_cref<O>::type(), typename no_cref<P>::type p = typename no_cref<P>::type())
		  : a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h), i(i), j(j), k(k), l(l), m(m), n(n), o(o), p(p)
	{}
	bool operator==(const base_tuple &bo) const {
		const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &to = (const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &)bo;
    return (comparer<A>::compare(a, to.a) &&
            comparer<B>::compare(b, to.b) &&
            comparer<C>::compare(c, to.c) &&
            comparer<D>::compare(d, to.d) &&
            comparer<E>::compare(e, to.e) &&
            comparer<F>::compare(f, to.f) &&
            comparer<G>::compare(g, to.g) &&
            comparer<H>::compare(h, to.h) &&
            comparer<I>::compare(i, to.i) &&
            comparer<J>::compare(j, to.j) &&
            comparer<K>::compare(k, to.k) &&
            comparer<L>::compare(l, to.l) &&
            comparer<M>::compare(m, to.m) &&
            comparer<N>::compare(n, to.n) &&
            comparer<O>::compare(o, to.o) &&
				    comparer<P>::compare(p, to.p));
	}
  virtual void printTo(std::ostream &os) const
  {
    os << "(";
    printArg<A>::print(os, a, false);
    printArg<B>::print(os, b, true);
    printArg<C>::print(os, c, true);
    printArg<D>::print(os, d, true);
    printArg<E>::print(os, e, true);
    printArg<F>::print(os, f, true);
    printArg<G>::print(os, g, true);
    printArg<H>::print(os, h, true);
    printArg<I>::print(os, i, true);
    printArg<J>::print(os, j, true);
    printArg<K>::print(os, k, true);
    printArg<L>::print(os, l, true);
    printArg<M>::print(os, m, true);
    printArg<N>::print(os, n, true);
    printArg<O>::print(os, o, true);
    printArg<P>::print(os, p, true);
    os << ")";
  }
};

class BaseException : public std::exception {
	char buffer[EXCEPTION_BUFFER_SIZE];
public:
	void setException(const char *description, MockRepository *repo);
	const char *what() const throw() { return buffer; }
};

// exception types
class ExpectationException : public BaseException {
public:
  ExpectationException(MockRepository *repo, const base_tuple *tuple, const char *funcName)
  {
  	std::stringstream text;
    text << "Function ";
    text << funcName;
    if (tuple)
      tuple->printTo(text);
    else
      text << "(...)";
    text << " called with mismatching expectation!" << std::endl;
    std::string description = text.str();
    setException(description.c_str(), repo);
  }
};

class NotImplementedException : public BaseException {
public:
	NotImplementedException(MockRepository *repo)
  {
    setException("Function called without expectation!", repo);
  }
};

class CallMissingException : public BaseException {
public:
	CallMissingException(MockRepository *repo)
  {
  	setException("Function with expectation not called!", repo);
  }
};

class NoResultSetUpException : public std::exception {
	char buffer[EXCEPTION_BUFFER_SIZE];
public:
	const char *what() const throw() { return buffer; }
  NoResultSetUpException(const base_tuple *tuple, const char *funcName)
  {
	  std::stringstream text;
    text << "No result set up on call to ";
    text << funcName;
    if (tuple)
      tuple->printTo(text);
    else
      text << "(...)";
    text << std::endl;
	  std::string result = text.str();
	  strncpy(buffer, result.c_str(), sizeof(buffer)-1);
  }
};

// function-index-of-type
class func_index {
public:
	virtual int f0() { return 0; }			virtual int f1() { return 1; }			virtual int f2() { return 2; }			virtual int f3() { return 3; }
	virtual int f4() { return 4; }			virtual int f5() { return 5; }			virtual int f6() { return 6; }			virtual int f7() { return 7; }
	virtual int f8() { return 8; }			virtual int f9() { return 9; }			virtual int f10() { return 10; }		virtual int f11() { return 11; }
	virtual int f12() { return 12; }		virtual int f13() { return 13; }		virtual int f14() { return 14; }		virtual int f15() { return 15; }
	virtual int f16() { return 16; }		virtual int f17() { return 17; }		virtual int f18() { return 18; }		virtual int f19() { return 19; }
	virtual int f20() { return 20; }		virtual int f21() { return 21; }		virtual int f22() { return 22; }		virtual int f23() { return 23; }
	virtual int f24() { return 24; }		virtual int f25() { return 25; }		virtual int f26() { return 26; }		virtual int f27() { return 27; }
	virtual int f28() { return 28; }		virtual int f29() { return 29; }		virtual int f30() { return 30; }		virtual int f31() { return 31; }
	virtual int f32() { return 32; }		virtual int f33() { return 33; }		virtual int f34() { return 34; }		virtual int f35() { return 35; }
	virtual int f36() { return 36; }		virtual int f37() { return 37; }		virtual int f38() { return 38; }		virtual int f39() { return 39; }
	virtual int f40() { return 40; }		virtual int f41() { return 41; }		virtual int f42() { return 42; }		virtual int f43() { return 43; }
	virtual int f44() { return 44; }		virtual int f45() { return 45; }		virtual int f46() { return 46; }		virtual int f47() { return 47; }
	virtual int f48() { return 48; }		virtual int f49() { return 49; }		virtual int f50() { return 50; }		virtual int f51() { return 51; }
	virtual int f52() { return 52; }		virtual int f53() { return 53; }		virtual int f54() { return 54; }		virtual int f55() { return 55; }
	virtual int f56() { return 56; }		virtual int f57() { return 57; }		virtual int f58() { return 58; }		virtual int f59() { return 59; }
	virtual int f60() { return 60; }		virtual int f61() { return 61; }		virtual int f62() { return 62; }		virtual int f63() { return 63; }
	virtual int f64() { return 64; }		virtual int f65() { return 65; }		virtual int f66() { return 66; }		virtual int f67() { return 67; }
	virtual int f68() { return 68; }		virtual int f69() { return 69; }		virtual int f70() { return 70; }		virtual int f71() { return 71; }
	virtual int f72() { return 72; }		virtual int f73() { return 73; }		virtual int f74() { return 74; }		virtual int f75() { return 75; }
	virtual int f76() { return 76; }		virtual int f77() { return 77; }		virtual int f78() { return 78; }		virtual int f79() { return 79; }
	virtual int f80() { return 80; }		virtual int f81() { return 81; }		virtual int f82() { return 82; }		virtual int f83() { return 83; }
	virtual int f84() { return 84; }		virtual int f85() { return 85; }		virtual int f86() { return 86; }		virtual int f87() { return 87; }
	virtual int f88() { return 88; }		virtual int f89() { return 89; }		virtual int f90() { return 90; }		virtual int f91() { return 91; }
	virtual int f92() { return 92; }		virtual int f93() { return 93; }		virtual int f94() { return 94; }		virtual int f95() { return 95; }
	virtual int f96() { return 96; }		virtual int f97() { return 97; }		virtual int f98() { return 98; }		virtual int f99() { return 99; }
	virtual int f100() { return 100; }		virtual int f101() { return 101; }		virtual int f102() { return 102; }		virtual int f103() { return 103; }
	virtual int f104() { return 104; }		virtual int f105() { return 105; }		virtual int f106() { return 106; }		virtual int f107() { return 107; }
	virtual int f108() { return 108; }		virtual int f109() { return 109; }		virtual int f110() { return 110; }		virtual int f111() { return 111; }
	virtual int f112() { return 112; }		virtual int f113() { return 113; }		virtual int f114() { return 114; }		virtual int f115() { return 115; }
	virtual int f116() { return 116; }		virtual int f117() { return 117; }		virtual int f118() { return 118; }		virtual int f119() { return 119; }
	virtual int f120() { return 120; }		virtual int f121() { return 121; }		virtual int f122() { return 122; }		virtual int f123() { return 123; }
	virtual int f124() { return 124; }		virtual int f125() { return 125; }		virtual int f126() { return 126; }		virtual int f127() { return 127; }
	virtual int f128() { return 128; }		virtual int f129() { return 129; }		virtual int f130() { return 130; }		virtual int f131() { return 131; }
	virtual int f132() { return 132; }		virtual int f133() { return 133; }		virtual int f134() { return 134; }		virtual int f135() { return 135; }
	virtual int f136() { return 136; }		virtual int f137() { return 137; }		virtual int f138() { return 138; }		virtual int f139() { return 139; }
	virtual int f140() { return 140; }		virtual int f141() { return 141; }		virtual int f142() { return 142; }		virtual int f143() { return 143; }
	virtual int f144() { return 144; }		virtual int f145() { return 145; }		virtual int f146() { return 146; }		virtual int f147() { return 147; }
	virtual int f148() { return 148; }		virtual int f149() { return 149; }		virtual int f150() { return 150; }		virtual int f151() { return 151; }
	virtual int f152() { return 152; }		virtual int f153() { return 153; }		virtual int f154() { return 154; }		virtual int f155() { return 155; }
	virtual int f156() { return 156; }		virtual int f157() { return 157; }		virtual int f158() { return 158; }		virtual int f159() { return 159; }
	virtual int f160() { return 160; }		virtual int f161() { return 161; }		virtual int f162() { return 162; }		virtual int f163() { return 163; }
	virtual int f164() { return 164; }		virtual int f165() { return 165; }		virtual int f166() { return 166; }		virtual int f167() { return 167; }
	virtual int f168() { return 168; }		virtual int f169() { return 169; }		virtual int f170() { return 170; }		virtual int f171() { return 171; }
	virtual int f172() { return 172; }		virtual int f173() { return 173; }		virtual int f174() { return 174; }		virtual int f175() { return 175; }
	virtual int f176() { return 176; }		virtual int f177() { return 177; }		virtual int f178() { return 178; }		virtual int f179() { return 179; }
	virtual int f180() { return 180; }		virtual int f181() { return 181; }		virtual int f182() { return 182; }		virtual int f183() { return 183; }
	virtual int f184() { return 184; }		virtual int f185() { return 185; }		virtual int f186() { return 186; }		virtual int f187() { return 187; }
	virtual int f188() { return 188; }		virtual int f189() { return 189; }		virtual int f190() { return 190; }		virtual int f191() { return 191; }
	virtual int f192() { return 192; }		virtual int f193() { return 193; }		virtual int f194() { return 194; }		virtual int f195() { return 195; }
	virtual int f196() { return 196; }		virtual int f197() { return 197; }		virtual int f198() { return 198; }		virtual int f199() { return 199; }
	virtual int f200() { return 200; }		virtual int f201() { return 201; }		virtual int f202() { return 202; }		virtual int f203() { return 203; }
	virtual int f204() { return 204; }		virtual int f205() { return 205; }		virtual int f206() { return 206; }		virtual int f207() { return 207; }
	virtual int f208() { return 208; }		virtual int f209() { return 209; }		virtual int f210() { return 210; }		virtual int f211() { return 211; }
	virtual int f212() { return 212; }		virtual int f213() { return 213; }		virtual int f214() { return 214; }		virtual int f215() { return 215; }
	virtual int f216() { return 216; }		virtual int f217() { return 217; }		virtual int f218() { return 218; }		virtual int f219() { return 219; }
	virtual int f220() { return 220; }		virtual int f221() { return 221; }		virtual int f222() { return 222; }		virtual int f223() { return 223; }
	virtual int f224() { return 224; }		virtual int f225() { return 225; }		virtual int f226() { return 226; }		virtual int f227() { return 227; }
	virtual int f228() { return 228; }		virtual int f229() { return 229; }		virtual int f230() { return 230; }		virtual int f231() { return 231; }
	virtual int f232() { return 232; }		virtual int f233() { return 233; }		virtual int f234() { return 234; }		virtual int f235() { return 235; }
	virtual int f236() { return 236; }		virtual int f237() { return 237; }		virtual int f238() { return 238; }		virtual int f239() { return 239; }
	virtual int f240() { return 240; }		virtual int f241() { return 241; }		virtual int f242() { return 242; }		virtual int f243() { return 243; }
	virtual int f244() { return 244; }		virtual int f245() { return 245; }		virtual int f246() { return 246; }		virtual int f247() { return 247; }
	virtual int f248() { return 248; }		virtual int f249() { return 249; }		virtual int f250() { return 250; }		virtual int f251() { return 251; }
	virtual int f252() { return 252; }		virtual int f253() { return 253; }		virtual int f254() { return 254; }		virtual int f255() { return 255; }
	virtual int f256() { return 256; }		virtual int f257() { return 257; }		virtual int f258() { return 258; }		virtual int f259() { return 259; }
	virtual int f260() { return 260; }		virtual int f261() { return 261; }		virtual int f262() { return 262; }		virtual int f263() { return 263; }
	virtual int f264() { return 264; }		virtual int f265() { return 265; }		virtual int f266() { return 266; }		virtual int f267() { return 267; }
	virtual int f268() { return 268; }		virtual int f269() { return 269; }		virtual int f270() { return 270; }		virtual int f271() { return 271; }
	virtual int f272() { return 272; }		virtual int f273() { return 273; }		virtual int f274() { return 274; }		virtual int f275() { return 275; }
	virtual int f276() { return 276; }		virtual int f277() { return 277; }		virtual int f278() { return 278; }		virtual int f279() { return 279; }
	virtual int f280() { return 280; }		virtual int f281() { return 281; }		virtual int f282() { return 282; }		virtual int f283() { return 283; }
	virtual int f284() { return 284; }		virtual int f285() { return 285; }		virtual int f286() { return 286; }		virtual int f287() { return 287; }
	virtual int f288() { return 288; }		virtual int f289() { return 289; }		virtual int f290() { return 290; }		virtual int f291() { return 291; }
	virtual int f292() { return 292; }		virtual int f293() { return 293; }		virtual int f294() { return 294; }		virtual int f295() { return 295; }
	virtual int f296() { return 296; }		virtual int f297() { return 297; }		virtual int f298() { return 298; }		virtual int f299() { return 299; }
	virtual int f300() { return 300; }		virtual int f301() { return 301; }		virtual int f302() { return 302; }		virtual int f303() { return 303; }
	virtual int f304() { return 304; }		virtual int f305() { return 305; }		virtual int f306() { return 306; }		virtual int f307() { return 307; }
	virtual int f308() { return 308; }		virtual int f309() { return 309; }		virtual int f310() { return 310; }		virtual int f311() { return 311; }
	virtual int f312() { return 312; }		virtual int f313() { return 313; }		virtual int f314() { return 314; }		virtual int f315() { return 315; }
	virtual int f316() { return 316; }		virtual int f317() { return 317; }		virtual int f318() { return 318; }		virtual int f319() { return 319; }
	virtual int f320() { return 320; }		virtual int f321() { return 321; }		virtual int f322() { return 322; }		virtual int f323() { return 323; }
	virtual int f324() { return 324; }		virtual int f325() { return 325; }		virtual int f326() { return 326; }		virtual int f327() { return 327; }
	virtual int f328() { return 328; }		virtual int f329() { return 329; }		virtual int f330() { return 330; }		virtual int f331() { return 331; }
	virtual int f332() { return 332; }		virtual int f333() { return 333; }		virtual int f334() { return 334; }		virtual int f335() { return 335; }
	virtual int f336() { return 336; }		virtual int f337() { return 337; }		virtual int f338() { return 338; }		virtual int f339() { return 339; }
	virtual int f340() { return 340; }		virtual int f341() { return 341; }		virtual int f342() { return 342; }		virtual int f343() { return 343; }
	virtual int f344() { return 344; }		virtual int f345() { return 345; }		virtual int f346() { return 346; }		virtual int f347() { return 347; }
	virtual int f348() { return 348; }		virtual int f349() { return 349; }		virtual int f350() { return 350; }		virtual int f351() { return 351; }
	virtual int f352() { return 352; }		virtual int f353() { return 353; }		virtual int f354() { return 354; }		virtual int f355() { return 355; }
	virtual int f356() { return 356; }		virtual int f357() { return 357; }		virtual int f358() { return 358; }		virtual int f359() { return 359; }
	virtual int f360() { return 360; }		virtual int f361() { return 361; }		virtual int f362() { return 362; }		virtual int f363() { return 363; }
	virtual int f364() { return 364; }		virtual int f365() { return 365; }		virtual int f366() { return 366; }		virtual int f367() { return 367; }
	virtual int f368() { return 368; }		virtual int f369() { return 369; }		virtual int f370() { return 370; }		virtual int f371() { return 371; }
	virtual int f372() { return 372; }		virtual int f373() { return 373; }		virtual int f374() { return 374; }		virtual int f375() { return 375; }
	virtual int f376() { return 376; }		virtual int f377() { return 377; }		virtual int f378() { return 378; }		virtual int f379() { return 379; }
	virtual int f380() { return 380; }		virtual int f381() { return 381; }		virtual int f382() { return 382; }		virtual int f383() { return 383; }
	virtual int f384() { return 384; }		virtual int f385() { return 385; }		virtual int f386() { return 386; }		virtual int f387() { return 387; }
	virtual int f388() { return 388; }		virtual int f389() { return 389; }		virtual int f390() { return 390; }		virtual int f391() { return 391; }
	virtual int f392() { return 392; }		virtual int f393() { return 393; }		virtual int f394() { return 394; }		virtual int f395() { return 395; }
	virtual int f396() { return 396; }		virtual int f397() { return 397; }		virtual int f398() { return 398; }		virtual int f399() { return 399; }
	virtual int f400() { return 400; }		virtual int f401() { return 401; }		virtual int f402() { return 402; }		virtual int f403() { return 403; }
	virtual int f404() { return 404; }		virtual int f405() { return 405; }		virtual int f406() { return 406; }		virtual int f407() { return 407; }
	virtual int f408() { return 408; }		virtual int f409() { return 409; }		virtual int f410() { return 410; }		virtual int f411() { return 411; }
	virtual int f412() { return 412; }		virtual int f413() { return 413; }		virtual int f414() { return 414; }		virtual int f415() { return 415; }
	virtual int f416() { return 416; }		virtual int f417() { return 417; }		virtual int f418() { return 418; }		virtual int f419() { return 419; }
	virtual int f420() { return 420; }		virtual int f421() { return 421; }		virtual int f422() { return 422; }		virtual int f423() { return 423; }
	virtual int f424() { return 424; }		virtual int f425() { return 425; }		virtual int f426() { return 426; }		virtual int f427() { return 427; }
	virtual int f428() { return 428; }		virtual int f429() { return 429; }		virtual int f430() { return 430; }		virtual int f431() { return 431; }
	virtual int f432() { return 432; }		virtual int f433() { return 433; }		virtual int f434() { return 434; }		virtual int f435() { return 435; }
	virtual int f436() { return 436; }		virtual int f437() { return 437; }		virtual int f438() { return 438; }		virtual int f439() { return 439; }
	virtual int f440() { return 440; }		virtual int f441() { return 441; }		virtual int f442() { return 442; }		virtual int f443() { return 443; }
	virtual int f444() { return 444; }		virtual int f445() { return 445; }		virtual int f446() { return 446; }		virtual int f447() { return 447; }
	virtual int f448() { return 448; }		virtual int f449() { return 449; }		virtual int f450() { return 450; }		virtual int f451() { return 451; }
	virtual int f452() { return 452; }		virtual int f453() { return 453; }		virtual int f454() { return 454; }		virtual int f455() { return 455; }
	virtual int f456() { return 456; }		virtual int f457() { return 457; }		virtual int f458() { return 458; }		virtual int f459() { return 459; }
	virtual int f460() { return 460; }		virtual int f461() { return 461; }		virtual int f462() { return 462; }		virtual int f463() { return 463; }
	virtual int f464() { return 464; }		virtual int f465() { return 465; }		virtual int f466() { return 466; }		virtual int f467() { return 467; }
	virtual int f468() { return 468; }		virtual int f469() { return 469; }		virtual int f470() { return 470; }		virtual int f471() { return 471; }
	virtual int f472() { return 472; }		virtual int f473() { return 473; }		virtual int f474() { return 474; }		virtual int f475() { return 475; }
	virtual int f476() { return 476; }		virtual int f477() { return 477; }		virtual int f478() { return 478; }		virtual int f479() { return 479; }
	virtual int f480() { return 480; }		virtual int f481() { return 481; }		virtual int f482() { return 482; }		virtual int f483() { return 483; }
	virtual int f484() { return 484; }		virtual int f485() { return 485; }		virtual int f486() { return 486; }		virtual int f487() { return 487; }
	virtual int f488() { return 488; }		virtual int f489() { return 489; }		virtual int f490() { return 490; }		virtual int f491() { return 491; }
	virtual int f492() { return 492; }		virtual int f493() { return 493; }		virtual int f494() { return 494; }		virtual int f495() { return 495; }
	virtual int f496() { return 496; }		virtual int f497() { return 497; }		virtual int f498() { return 498; }		virtual int f499() { return 499; }
	virtual int f500() { return 500; }		virtual int f501() { return 501; }		virtual int f502() { return 502; }		virtual int f503() { return 503; }
	virtual int f504() { return 504; }		virtual int f505() { return 505; }		virtual int f506() { return 506; }		virtual int f507() { return 507; }
	virtual int f508() { return 508; }		virtual int f509() { return 509; }		virtual int f510() { return 510; }		virtual int f511() { return 511; }
	virtual int f512() { return 512; }		virtual int f513() { return 513; }		virtual int f514() { return 514; }		virtual int f515() { return 515; }
	virtual int f516() { return 516; }		virtual int f517() { return 517; }		virtual int f518() { return 518; }		virtual int f519() { return 519; }
	virtual int f520() { return 520; }		virtual int f521() { return 521; }		virtual int f522() { return 522; }		virtual int f523() { return 523; }
	virtual int f524() { return 524; }		virtual int f525() { return 525; }		virtual int f526() { return 526; }		virtual int f527() { return 527; }
	virtual int f528() { return 528; }		virtual int f529() { return 529; }		virtual int f530() { return 530; }		virtual int f531() { return 531; }
	virtual int f532() { return 532; }		virtual int f533() { return 533; }		virtual int f534() { return 534; }		virtual int f535() { return 535; }
	virtual int f536() { return 536; }		virtual int f537() { return 537; }		virtual int f538() { return 538; }		virtual int f539() { return 539; }
	virtual int f540() { return 540; }		virtual int f541() { return 541; }		virtual int f542() { return 542; }		virtual int f543() { return 543; }
	virtual int f544() { return 544; }		virtual int f545() { return 545; }		virtual int f546() { return 546; }		virtual int f547() { return 547; }
	virtual int f548() { return 548; }		virtual int f549() { return 549; }		virtual int f550() { return 550; }		virtual int f551() { return 551; }
	virtual int f552() { return 552; }		virtual int f553() { return 553; }		virtual int f554() { return 554; }		virtual int f555() { return 555; }
	virtual int f556() { return 556; }		virtual int f557() { return 557; }		virtual int f558() { return 558; }		virtual int f559() { return 559; }
	virtual int f560() { return 560; }		virtual int f561() { return 561; }		virtual int f562() { return 562; }		virtual int f563() { return 563; }
	virtual int f564() { return 564; }		virtual int f565() { return 565; }		virtual int f566() { return 566; }		virtual int f567() { return 567; }
	virtual int f568() { return 568; }		virtual int f569() { return 569; }		virtual int f570() { return 570; }		virtual int f571() { return 571; }
	virtual int f572() { return 572; }		virtual int f573() { return 573; }		virtual int f574() { return 574; }		virtual int f575() { return 575; }
	virtual int f576() { return 576; }		virtual int f577() { return 577; }		virtual int f578() { return 578; }		virtual int f579() { return 579; }
	virtual int f580() { return 580; }		virtual int f581() { return 581; }		virtual int f582() { return 582; }		virtual int f583() { return 583; }
	virtual int f584() { return 584; }		virtual int f585() { return 585; }		virtual int f586() { return 586; }		virtual int f587() { return 587; }
	virtual int f588() { return 588; }		virtual int f589() { return 589; }		virtual int f590() { return 590; }		virtual int f591() { return 591; }
	virtual int f592() { return 592; }		virtual int f593() { return 593; }		virtual int f594() { return 594; }		virtual int f595() { return 595; }
	virtual int f596() { return 596; }		virtual int f597() { return 597; }		virtual int f598() { return 598; }		virtual int f599() { return 599; }
	virtual int f600() { return 600; }		virtual int f601() { return 601; }		virtual int f602() { return 602; }		virtual int f603() { return 603; }
	virtual int f604() { return 604; }		virtual int f605() { return 605; }		virtual int f606() { return 606; }		virtual int f607() { return 607; }
	virtual int f608() { return 608; }		virtual int f609() { return 609; }		virtual int f610() { return 610; }		virtual int f611() { return 611; }
	virtual int f612() { return 612; }		virtual int f613() { return 613; }		virtual int f614() { return 614; }		virtual int f615() { return 615; }
	virtual int f616() { return 616; }		virtual int f617() { return 617; }		virtual int f618() { return 618; }		virtual int f619() { return 619; }
	virtual int f620() { return 620; }		virtual int f621() { return 621; }		virtual int f622() { return 622; }		virtual int f623() { return 623; }
	virtual int f624() { return 624; }		virtual int f625() { return 625; }		virtual int f626() { return 626; }		virtual int f627() { return 627; }
	virtual int f628() { return 628; }		virtual int f629() { return 629; }		virtual int f630() { return 630; }		virtual int f631() { return 631; }
	virtual int f632() { return 632; }		virtual int f633() { return 633; }		virtual int f634() { return 634; }		virtual int f635() { return 635; }
	virtual int f636() { return 636; }		virtual int f637() { return 637; }		virtual int f638() { return 638; }		virtual int f639() { return 639; }
	virtual int f640() { return 640; }		virtual int f641() { return 641; }		virtual int f642() { return 642; }		virtual int f643() { return 643; }
	virtual int f644() { return 644; }		virtual int f645() { return 645; }		virtual int f646() { return 646; }		virtual int f647() { return 647; }
	virtual int f648() { return 648; }		virtual int f649() { return 649; }		virtual int f650() { return 650; }		virtual int f651() { return 651; }
	virtual int f652() { return 652; }		virtual int f653() { return 653; }		virtual int f654() { return 654; }		virtual int f655() { return 655; }
	virtual int f656() { return 656; }		virtual int f657() { return 657; }		virtual int f658() { return 658; }		virtual int f659() { return 659; }
	virtual int f660() { return 660; }		virtual int f661() { return 661; }		virtual int f662() { return 662; }		virtual int f663() { return 663; }
	virtual int f664() { return 664; }		virtual int f665() { return 665; }		virtual int f666() { return 666; }		virtual int f667() { return 667; }
	virtual int f668() { return 668; }		virtual int f669() { return 669; }		virtual int f670() { return 670; }		virtual int f671() { return 671; }
	virtual int f672() { return 672; }		virtual int f673() { return 673; }		virtual int f674() { return 674; }		virtual int f675() { return 675; }
	virtual int f676() { return 676; }		virtual int f677() { return 677; }		virtual int f678() { return 678; }		virtual int f679() { return 679; }
	virtual int f680() { return 680; }		virtual int f681() { return 681; }		virtual int f682() { return 682; }		virtual int f683() { return 683; }
	virtual int f684() { return 684; }		virtual int f685() { return 685; }		virtual int f686() { return 686; }		virtual int f687() { return 687; }
	virtual int f688() { return 688; }		virtual int f689() { return 689; }		virtual int f690() { return 690; }		virtual int f691() { return 691; }
	virtual int f692() { return 692; }		virtual int f693() { return 693; }		virtual int f694() { return 694; }		virtual int f695() { return 695; }
	virtual int f696() { return 696; }		virtual int f697() { return 697; }		virtual int f698() { return 698; }		virtual int f699() { return 699; }
	virtual int f700() { return 700; }		virtual int f701() { return 701; }		virtual int f702() { return 702; }		virtual int f703() { return 703; }
	virtual int f704() { return 704; }		virtual int f705() { return 705; }		virtual int f706() { return 706; }		virtual int f707() { return 707; }
	virtual int f708() { return 708; }		virtual int f709() { return 709; }		virtual int f710() { return 710; }		virtual int f711() { return 711; }
	virtual int f712() { return 712; }		virtual int f713() { return 713; }		virtual int f714() { return 714; }		virtual int f715() { return 715; }
	virtual int f716() { return 716; }		virtual int f717() { return 717; }		virtual int f718() { return 718; }		virtual int f719() { return 719; }
	virtual int f720() { return 720; }		virtual int f721() { return 721; }		virtual int f722() { return 722; }		virtual int f723() { return 723; }
	virtual int f724() { return 724; }		virtual int f725() { return 725; }		virtual int f726() { return 726; }		virtual int f727() { return 727; }
	virtual int f728() { return 728; }		virtual int f729() { return 729; }		virtual int f730() { return 730; }		virtual int f731() { return 731; }
	virtual int f732() { return 732; }		virtual int f733() { return 733; }		virtual int f734() { return 734; }		virtual int f735() { return 735; }
	virtual int f736() { return 736; }		virtual int f737() { return 737; }		virtual int f738() { return 738; }		virtual int f739() { return 739; }
	virtual int f740() { return 740; }		virtual int f741() { return 741; }		virtual int f742() { return 742; }		virtual int f743() { return 743; }
	virtual int f744() { return 744; }		virtual int f745() { return 745; }		virtual int f746() { return 746; }		virtual int f747() { return 747; }
	virtual int f748() { return 748; }		virtual int f749() { return 749; }		virtual int f750() { return 750; }		virtual int f751() { return 751; }
	virtual int f752() { return 752; }		virtual int f753() { return 753; }		virtual int f754() { return 754; }		virtual int f755() { return 755; }
	virtual int f756() { return 756; }		virtual int f757() { return 757; }		virtual int f758() { return 758; }		virtual int f759() { return 759; }
	virtual int f760() { return 760; }		virtual int f761() { return 761; }		virtual int f762() { return 762; }		virtual int f763() { return 763; }
	virtual int f764() { return 764; }		virtual int f765() { return 765; }		virtual int f766() { return 766; }		virtual int f767() { return 767; }
	virtual int f768() { return 768; }		virtual int f769() { return 769; }		virtual int f770() { return 770; }		virtual int f771() { return 771; }
	virtual int f772() { return 772; }		virtual int f773() { return 773; }		virtual int f774() { return 774; }		virtual int f775() { return 775; }
	virtual int f776() { return 776; }		virtual int f777() { return 777; }		virtual int f778() { return 778; }		virtual int f779() { return 779; }
	virtual int f780() { return 780; }		virtual int f781() { return 781; }		virtual int f782() { return 782; }		virtual int f783() { return 783; }
	virtual int f784() { return 784; }		virtual int f785() { return 785; }		virtual int f786() { return 786; }		virtual int f787() { return 787; }
	virtual int f788() { return 788; }		virtual int f789() { return 789; }		virtual int f790() { return 790; }		virtual int f791() { return 791; }
	virtual int f792() { return 792; }		virtual int f793() { return 793; }		virtual int f794() { return 794; }		virtual int f795() { return 795; }
	virtual int f796() { return 796; }		virtual int f797() { return 797; }		virtual int f798() { return 798; }		virtual int f799() { return 799; }
	virtual int f800() { return 800; }		virtual int f801() { return 801; }		virtual int f802() { return 802; }		virtual int f803() { return 803; }
	virtual int f804() { return 804; }		virtual int f805() { return 805; }		virtual int f806() { return 806; }		virtual int f807() { return 807; }
	virtual int f808() { return 808; }		virtual int f809() { return 809; }		virtual int f810() { return 810; }		virtual int f811() { return 811; }
	virtual int f812() { return 812; }		virtual int f813() { return 813; }		virtual int f814() { return 814; }		virtual int f815() { return 815; }
	virtual int f816() { return 816; }		virtual int f817() { return 817; }		virtual int f818() { return 818; }		virtual int f819() { return 819; }
	virtual int f820() { return 820; }		virtual int f821() { return 821; }		virtual int f822() { return 822; }		virtual int f823() { return 823; }
	virtual int f824() { return 824; }		virtual int f825() { return 825; }		virtual int f826() { return 826; }		virtual int f827() { return 827; }
	virtual int f828() { return 828; }		virtual int f829() { return 829; }		virtual int f830() { return 830; }		virtual int f831() { return 831; }
	virtual int f832() { return 832; }		virtual int f833() { return 833; }		virtual int f834() { return 834; }		virtual int f835() { return 835; }
	virtual int f836() { return 836; }		virtual int f837() { return 837; }		virtual int f838() { return 838; }		virtual int f839() { return 839; }
	virtual int f840() { return 840; }		virtual int f841() { return 841; }		virtual int f842() { return 842; }		virtual int f843() { return 843; }
	virtual int f844() { return 844; }		virtual int f845() { return 845; }		virtual int f846() { return 846; }		virtual int f847() { return 847; }
	virtual int f848() { return 848; }		virtual int f849() { return 849; }		virtual int f850() { return 850; }		virtual int f851() { return 851; }
	virtual int f852() { return 852; }		virtual int f853() { return 853; }		virtual int f854() { return 854; }		virtual int f855() { return 855; }
	virtual int f856() { return 856; }		virtual int f857() { return 857; }		virtual int f858() { return 858; }		virtual int f859() { return 859; }
	virtual int f860() { return 860; }		virtual int f861() { return 861; }		virtual int f862() { return 862; }		virtual int f863() { return 863; }
	virtual int f864() { return 864; }		virtual int f865() { return 865; }		virtual int f866() { return 866; }		virtual int f867() { return 867; }
	virtual int f868() { return 868; }		virtual int f869() { return 869; }		virtual int f870() { return 870; }		virtual int f871() { return 871; }
	virtual int f872() { return 872; }		virtual int f873() { return 873; }		virtual int f874() { return 874; }		virtual int f875() { return 875; }
	virtual int f876() { return 876; }		virtual int f877() { return 877; }		virtual int f878() { return 878; }		virtual int f879() { return 879; }
	virtual int f880() { return 880; }		virtual int f881() { return 881; }		virtual int f882() { return 882; }		virtual int f883() { return 883; }
	virtual int f884() { return 884; }		virtual int f885() { return 885; }		virtual int f886() { return 886; }		virtual int f887() { return 887; }
	virtual int f888() { return 888; }		virtual int f889() { return 889; }		virtual int f890() { return 890; }		virtual int f891() { return 891; }
	virtual int f892() { return 892; }		virtual int f893() { return 893; }		virtual int f894() { return 894; }		virtual int f895() { return 895; }
	virtual int f896() { return 896; }		virtual int f897() { return 897; }		virtual int f898() { return 898; }		virtual int f899() { return 899; }
	virtual int f900() { return 900; }		virtual int f901() { return 901; }		virtual int f902() { return 902; }		virtual int f903() { return 903; }
	virtual int f904() { return 904; }		virtual int f905() { return 905; }		virtual int f906() { return 906; }		virtual int f907() { return 907; }
	virtual int f908() { return 908; }		virtual int f909() { return 909; }		virtual int f910() { return 910; }		virtual int f911() { return 911; }
	virtual int f912() { return 912; }		virtual int f913() { return 913; }		virtual int f914() { return 914; }		virtual int f915() { return 915; }
	virtual int f916() { return 916; }		virtual int f917() { return 917; }		virtual int f918() { return 918; }		virtual int f919() { return 919; }
	virtual int f920() { return 920; }		virtual int f921() { return 921; }		virtual int f922() { return 922; }		virtual int f923() { return 923; }
	virtual int f924() { return 924; }		virtual int f925() { return 925; }		virtual int f926() { return 926; }		virtual int f927() { return 927; }
	virtual int f928() { return 928; }		virtual int f929() { return 929; }		virtual int f930() { return 930; }		virtual int f931() { return 931; }
	virtual int f932() { return 932; }		virtual int f933() { return 933; }		virtual int f934() { return 934; }		virtual int f935() { return 935; }
	virtual int f936() { return 936; }		virtual int f937() { return 937; }		virtual int f938() { return 938; }		virtual int f939() { return 939; }
	virtual int f940() { return 940; }		virtual int f941() { return 941; }		virtual int f942() { return 942; }		virtual int f943() { return 943; }
	virtual int f944() { return 944; }		virtual int f945() { return 945; }		virtual int f946() { return 946; }		virtual int f947() { return 947; }
	virtual int f948() { return 948; }		virtual int f949() { return 949; }		virtual int f950() { return 950; }		virtual int f951() { return 951; }
	virtual int f952() { return 952; }		virtual int f953() { return 953; }		virtual int f954() { return 954; }		virtual int f955() { return 955; }
	virtual int f956() { return 956; }		virtual int f957() { return 957; }		virtual int f958() { return 958; }		virtual int f959() { return 959; }
	virtual int f960() { return 960; }		virtual int f961() { return 961; }		virtual int f962() { return 962; }		virtual int f963() { return 963; }
	virtual int f964() { return 964; }		virtual int f965() { return 965; }		virtual int f966() { return 966; }		virtual int f967() { return 967; }
	virtual int f968() { return 968; }		virtual int f969() { return 969; }		virtual int f970() { return 970; }		virtual int f971() { return 971; }
	virtual int f972() { return 972; }		virtual int f973() { return 973; }		virtual int f974() { return 974; }		virtual int f975() { return 975; }
	virtual int f976() { return 976; }		virtual int f977() { return 977; }		virtual int f978() { return 978; }		virtual int f979() { return 979; }
	virtual int f980() { return 980; }		virtual int f981() { return 981; }		virtual int f982() { return 982; }		virtual int f983() { return 983; }
	virtual int f984() { return 984; }		virtual int f985() { return 985; }		virtual int f986() { return 986; }		virtual int f987() { return 987; }
	virtual int f988() { return 988; }		virtual int f989() { return 989; }		virtual int f990() { return 990; }		virtual int f991() { return 991; }
	virtual int f992() { return 992; }		virtual int f993() { return 993; }		virtual int f994() { return 994; }		virtual int f995() { return 995; }
	virtual int f996() { return 996; }		virtual int f997() { return 997; }		virtual int f998() { return 998; }		virtual int f999() { return 999; }
	virtual int f1000() { return 1000; }	virtual int f1001() { return 1001; }	virtual int f1002() { return 1002; }	virtual int f1003() { return 1003; }
	virtual int f1004() { return 1004; }	virtual int f1005() { return 1005; }	virtual int f1006() { return 1006; }	virtual int f1007() { return 1007; }
	virtual int f1008() { return 1008; }	virtual int f1009() { return 1009; }	virtual int f1010() { return 1010; }	virtual int f1011() { return 1011; }
	virtual int f1012() { return 1012; }	virtual int f1013() { return 1013; }	virtual int f1014() { return 1014; }	virtual int f1015() { return 1015; }
	virtual int f1016() { return 1016; }	virtual int f1017() { return 1017; }	virtual int f1018() { return 1018; }	virtual int f1019() { return 1019; }
	virtual int f1020() { return 1020; }	virtual int f1021() { return 1021; }	virtual int f1022() { return 1022; }	virtual int f1023() { return 1023; }
};

template <typename T, typename U>
T getNonvirtualMemberFunctionAddress(U u)
{
#ifdef __EDG__
  // Edison Design Group C++ frontend (Comeau, Portland Group, Greenhills, etc)
  union {
    struct {
      short delta;
      short vindex;
      T t;
    } mfp_structure;
    U u;
  } conv;
#else
  // Visual Studio, GCC, others
  union {
    struct {
      T t;
    } mfp_structure;
    U u;
  } conv;
#endif
  conv.u = u;
  return conv.mfp_structure.t;
}

template <typename T>
int getFunctionIndex(T func) {
	func_index idx;
	return ((&idx)->*reinterpret_cast<int (func_index::*)()>(func))() * FUNCTION_STRIDE + FUNCTION_BASE;
}

// mock types
template <class T>
class mock : public base_mock 
{
	friend class MockRepository;
	unsigned char remaining[sizeof(T)];
	void NotImplemented() { throw NotImplementedException(repo); }
protected:
	void *oldVft;
	void (*funcs[VIRT_FUNC_LIMIT])();
	MockRepository *repo;
public:
	int funcMap[VIRT_FUNC_LIMIT];
	mock(MockRepository *repo) 
		: repo(repo)
	{
		for (int i = 0; i < VIRT_FUNC_LIMIT; i++) 
		{
			funcs[i] = getNonvirtualMemberFunctionAddress<void (*)()>(&mock<T>::NotImplemented);
			funcMap[i] = -1;
		}
		memset(remaining, 0, sizeof(remaining));
		oldVft = base_mock::rewriteVft(funcs);
	}
	int translateX(int x) 
	{
		for (int i = 0; i < VIRT_FUNC_LIMIT; i++) 
		{
			if (funcMap[i] == x) return i;
		}
		return -1;
	}
};

template <class T>
class classMock : public mock<T>
{
	void *backupVft;
public:
	classMock(MockRepository *repo) 
		: mock<T>(repo)
	{
		mock<T>::oldVft = base_mock::rewriteVft((void *)mock<T>::funcs);
		new(this)T();
		backupVft = base_mock::rewriteVft((void *)mock<T>::funcs);
	}
	~classMock()
	{
		base_mock::rewriteVft(backupVft);
		((T *)this)->~T();
	}
};

//Type-safe exception wrapping
class ExceptionHolder 
{ 
public:
	virtual ~ExceptionHolder() {}
	virtual void rethrow() = 0; 
};

template <class T>
class ExceptionWrapper : public ExceptionHolder {
	T exception;
public:
	ExceptionWrapper(T exception) : exception(exception) {}
	void rethrow() { throw exception; }
};

// Do() function wrapping
class VirtualDestructable { public: virtual ~VirtualDestructable() {} };

template <typename Y>
class TupleInvocable : public VirtualDestructable 
{
public:
	virtual Y operator()(const base_tuple &tupl) = 0;
};

template <typename Y,
		  typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType, 
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType, 
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType, 
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class Invocable : public TupleInvocable<Y>
{
public:
	virtual Y operator()(A a = A(), B b = B(), C c = C(), D d = D(), E e = E(), F f = F(), G g = G(), H h = H(), I i = I(), J j = J(), K k = K(), L l = L(), M m = M(), N n = N(), O o = O(), P p = P()) = 0;
	virtual Y operator()(const base_tuple &tupl) {
		const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &rTupl = reinterpret_cast<const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl);
		return (*this)(rTupl.a, rTupl.b, rTupl.c, rTupl.d, rTupl.e, rTupl.f, rTupl.g, rTupl.h, 
			rTupl.i, rTupl.j, rTupl.k, rTupl.l, rTupl.m, rTupl.n, rTupl.o, rTupl.p);
	}
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J, typename K, typename L, 
		  typename M, typename N, typename O, typename P>
class DoWrapper : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J, typename K, typename L, 
		  typename M, typename N, typename O>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J, typename K, typename L, 
		  typename M, typename N>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m,n);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J, typename K, typename L, 
		  typename M>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l,m);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L,M>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J, typename K, typename L>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k,l);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K,L>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J, typename K>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J,K> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j,k);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J,K>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H,I,J> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i,j);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I,J>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,NullType,NullType,NullType,NullType,NullType,NullType,NullType>  : public Invocable<Y,A,B,C,D,E,F,G,H,I>{
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, I i, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h,i);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H,I>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H>
class DoWrapper<T,Y,A,B,C,D,E,F,G,H,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G,H> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, H h, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g,h);
	}
	using Invocable<Y,A,B,C,D,E,F,G,H>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G>
class DoWrapper<T,Y,A,B,C,D,E,F,G,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F,G> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, G g, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f,g);
	}
	using Invocable<Y,A,B,C,D,E,F,G>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F>
class DoWrapper<T,Y,A,B,C,D,E,F,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E,F> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, F f, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e,f);
	}
	using Invocable<Y,A,B,C,D,E,F>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D, 
		  typename E>
class DoWrapper<T,Y,A,B,C,D,E,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D,E> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, E e, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d,e);
	}
	using Invocable<Y,A,B,C,D,E>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C, typename D>
class DoWrapper<T,Y,A,B,C,D,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C,D> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, D d, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c,d);
	}
	using Invocable<Y,A,B,C,D>::operator();
};
template <typename T, typename Y,
		  typename A, typename B, typename C>
class DoWrapper<T,Y,A,B,C,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B,C> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, C c, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b,c);
	}
	using Invocable<Y,A,B,C>::operator();
};
template <typename T, typename Y, typename A, typename B>
class DoWrapper<T,Y,A,B,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A,B> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, B b, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a,b);
	}
	using Invocable<Y,A,B>::operator();
};
template <typename T, typename Y, typename A>
class DoWrapper<T,Y,A,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y,A> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(A a, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t(a);
	}
	using Invocable<Y,A>::operator();
};
template <typename T, typename Y>
class DoWrapper<T,Y,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType,NullType> : public Invocable<Y> {
	T &t;
public:
	DoWrapper(T &t) : t(t) {}
	virtual Y operator()(NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType, NullType)
	{
		return t();
	}
	using Invocable<Y>::operator();
};

class ReturnValueHolder {
public:
	virtual ~ReturnValueHolder() {}
};

template <class T>
class ReturnValueWrapper : public ReturnValueHolder {
public:
	typename no_cref<T>::type rv;
	ReturnValueWrapper(T rv) : rv(rv) {}
};

//Call wrapping
class Call {
public:
	virtual bool matchesArgs(const base_tuple &tuple) = 0;
	ReturnValueHolder *retVal;
	ExceptionHolder *eHolder;
	base_mock *mock;
	VirtualDestructable *functor;
	int funcIndex;
	std::list<Call *> previousCalls;
	RegistrationType expectation;
	bool satisfied;
	int lineno;
	const char *funcName;
	const char *fileName;
protected:
	Call(RegistrationType expectation, base_mock *mock, int funcIndex, int X, const char *funcName, const char *fileName) 
		: retVal(0), 
		eHolder(0), 
		mock(mock), 
		functor(0),
		funcIndex(funcIndex), 
		expectation(expectation),
		satisfied(false),
		lineno(X),
		funcName(funcName),
		fileName(fileName)
	{
	}
public:
  virtual const base_tuple *getArgs() const = 0;
	virtual ~Call() 
	{
		delete eHolder;
		delete functor;
		delete retVal;
	}
};

std::ostream &operator<<(std::ostream &os, const Call &call);

template <typename Y, 
		  typename A = NullType, typename B = NullType, typename C = NullType, typename D = NullType, 
		  typename E = NullType, typename F = NullType, typename G = NullType, typename H = NullType, 
		  typename I = NullType, typename J = NullType, typename K = NullType, typename L = NullType, 
		  typename M = NullType, typename N = NullType, typename O = NullType, typename P = NullType>
class TCall : public Call {
private:
	tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *args;
public:
    const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expectation, base_mock *mock, int funcIndex, int X, const char *funcName, const char *fileName) : Call(expectation, mock, funcIndex, X, funcName, fileName), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) { return !args || *args == reinterpret_cast<const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl); }
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &With(A a = A(), B b = B(), C c = C(), D d = D(), E e = E(), F f = F(), G g = G(), H h = H(), I i = I(), J j = J(), K k = K(), L l = L(), M m = M(), N n = N(), O o = O(), P p = P()) { 
		args = new tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p); 
		return *this; 
	}
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &After(Call &call) { 
		previousCalls.push_back(&call);
		return *this; 
	}
	template <typename T>
	Call &Do(T &function) { functor = new DoWrapper<T,Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(function); return *this; }
	Call &Return(Y obj) { retVal = new ReturnValueWrapper<Y>(obj); return *this; }
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
};

template <typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H, 
		  typename I, typename J, typename K, typename L, 
		  typename M, typename N, typename O, typename P>
class TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> : public Call {
private:
	tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *args;
public:
  const base_tuple *getArgs() const { return args; }
	TCall(RegistrationType expectation, base_mock *mock, int funcIndex, int X, const char *funcName, const char *fileName) : Call(expectation, mock, funcIndex, X, funcName, fileName), args(0) {}
	~TCall() { delete args; }
	bool matchesArgs(const base_tuple &tupl) { return (!args) || (*args == reinterpret_cast<const tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &>(tupl)); }
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &With(A a = A(), B b = B(), C c = C(), D d = D(), E e = E(), F f = F(), G g = G(), H h = H(), I i = I(), J j = J(), K k = K(), L l = L(), M m = M(), N n = N(), O o = O(), P p = P()) { 
		args = new tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p); 
		return *this; 
	}
	TCall<void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &After(Call &call) { 
		previousCalls.push_back(&call);
		return *this; 
	}
	template <typename T>
	Call &Do(T &function) { functor = new DoWrapper<T,void,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(function); return *this; }
	template <typename Ex>
	Call &Throw(Ex exception) { eHolder = new ExceptionWrapper<Ex>(exception); return *this; }
};

class MockRepository {
private:
	friend inline std::ostream &operator<<(std::ostream &os, const MockRepository &repo);
	std::list<base_mock *> mocks;
	std::list<Call *> neverCalls;
  std::list<Call *> expectations;
	std::list<Call *> optionals;
public:
	bool autoExpect;
#ifdef _MSC_VER
#define OnCall(obj, func) RegisterExpect_<__COUNTER__, DontCare>(obj, &func, #func, __FILE__, __LINE__)
#define ExpectCall(obj, func) RegisterExpect_<__COUNTER__, Once>(obj, &func, #func, __FILE__, __LINE__)
#define NeverCall(obj, func) RegisterExpect_<__COUNTER__, Never>(obj, &func, #func, __FILE__, __LINE__)
#define OnCallOverload(obj, func) RegisterExpect_<__COUNTER__, DontCare>(obj, func, #func, __FILE__, __LINE__)
#define ExpectCallOverload(obj, func) RegisterExpect_<__COUNTER__, Once>(obj, func, #func, __FILE__, __LINE__)
#define NeverCallOverload(obj, func) RegisterExpect_<__COUNTER__, Never>(obj, func, #func, __FILE__, __LINE__)
#else
#define OnCall(obj, func) RegisterExpect_<__LINE__, DontCare>(obj, &func, #func, __FILE__, __LINE__)
#define ExpectCall(obj, func) RegisterExpect_<__LINE__, Once>(obj, &func, #func, __FILE__, __LINE__)
#define NeverCall(obj, func) RegisterExpect_<__LINE__, Never>(obj, &func, #func, __FILE__, __LINE__)
#define OnCallOverload(obj, func) RegisterExpect_<__LINE__, DontCare>(obj, func, #func, __FILE__, __LINE__)
#define ExpectCallOverload(obj, func) RegisterExpect_<__LINE__, Once>(obj, func, #func, __FILE__, __LINE__)
#define NeverCallOverload(obj, func) RegisterExpect_<__LINE__, Never>(obj, func, #func, __FILE__, __LINE__)
#endif
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)(), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), const char *funcName, const char *fileName, unsigned long lineNo);
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), const char *funcName, const char *fileName, unsigned long lineNo);

	//GCC 3.x doesn't seem to understand overloading on const or non-const member function. Volatile appears to work though.
#if !defined(__GNUC__) || __GNUC__ > 3
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)() volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)())(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), funcName, fileName, lineNo); }

	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)() const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)())(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) const volatile, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), funcName, fileName, lineNo); }

	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z>
	TCall<Y> &RegisterExpect_(Z2 *mck, Y (Z::*func)() const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)())(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, typename A>
	TCall<Y,A> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B>
	TCall<Y,A,B> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C>
	TCall<Y,A,B,C> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D>
	TCall<Y,A,B,C,D> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E>
	TCall<Y,A,B,C,D,E> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F>
	TCall<Y,A,B,C,D,E,F> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G>
	TCall<Y,A,B,C,D,E,F,G> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H>
	TCall<Y,A,B,C,D,E,F,G,H> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I>
	TCall<Y,A,B,C,D,E,F,G,H,I> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J>
	TCall<Y,A,B,C,D,E,F,G,H,I,J> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O))(func), funcName, fileName, lineNo); }
	template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
			  typename A, typename B, typename C, typename D, 
			  typename E, typename F, typename G, typename H,
			  typename I, typename J, typename K, typename L,
			  typename M, typename N, typename O, typename P>
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) const, const char *funcName, const char *fileName, unsigned long lineNo) { return RegisterExpect_<X,expect>(mck, (Y(Z::*)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P))(func), funcName, fileName, lineNo); }
#endif
	template <typename Z>
	void BasicRegisterExpect(mock<Z> *zMock, int funcIndex, void (base_mock::*func)(), int X);
	template <typename Z>
	Z DoExpectation(base_mock *mock, int funcno, const base_tuple &tuple);
    void DoVoidExpectation(base_mock *mock, int funcno, const base_tuple &tuple) 
    {
		for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end(); ++i) 
		{
			Call *call = *i;
			if (call->mock == mock &&
				call->funcIndex == funcno &&
				call->matchesArgs(tuple) &&
				!call->satisfied)
			{
				bool allSatisfy = true;
				for (std::list<Call *>::iterator callsBefore = call->previousCalls.begin();
					callsBefore != call->previousCalls.end(); ++callsBefore)
				{
					if (!(*callsBefore)->satisfied)
					{
						allSatisfy = false;
					}
				}
				if (!allSatisfy) continue;

				call->satisfied = true;
					
				if (call->eHolder)
					call->eHolder->rethrow();

    			if (call->functor != NULL)
    				(*(TupleInvocable<void> *)(call->functor))(tuple);

				return;
	    	}
		}
		for (std::list<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end(); ++i) 
		{
			Call *call = *i;
			if (call->mock == mock &&
				call->funcIndex == funcno &&
				call->matchesArgs(tuple))
			{
				bool allSatisfy = true;
				for (std::list<Call *>::iterator callsBefore = call->previousCalls.begin();
					callsBefore != call->previousCalls.end(); ++callsBefore)
				{
					if (!(*callsBefore)->satisfied)
					{
						allSatisfy = false;
					}
				}
				if (!allSatisfy) continue;

				call->satisfied = true;

				throw ExpectationException(this, call->getArgs(), call->funcName);
			}
		}
		for (std::list<Call *>::iterator i = optionals.begin(); i != optionals.end(); ++i) 
		{
			Call *call = *i;
			if (call->mock == mock &&
				call->funcIndex == funcno &&
				call->matchesArgs(tuple))
			{
				bool allSatisfy = true;
				for (std::list<Call *>::iterator callsBefore = call->previousCalls.begin();
					callsBefore != call->previousCalls.end(); ++callsBefore)
				{
					if (!(*callsBefore)->satisfied)
					{
						allSatisfy = false;
					}
				}
				if (!allSatisfy) continue;

				call->satisfied = true;

				if (call->eHolder)
					call->eHolder->rethrow();

            	if (call->functor != NULL)
            		(*(TupleInvocable<void> *)(call->functor))(tuple);

        		return;
			}
		}
		const char *funcName = NULL;
  		for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end() && !funcName; ++i) 
	  	{
			Call *call = *i;
			if (call->mock == mock &&
				call->funcIndex == funcno)
			funcName = call->funcName;
		}
  		for (std::list<Call *>::iterator i = optionals.begin(); i != optionals.end() && !funcName; ++i) 
		{
			Call *call = *i;
			if (call->mock == mock &&
				call->funcIndex == funcno)
			funcName = call->funcName;
		}
  		for (std::list<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end() && !funcName; ++i) 
		{
			Call *call = *i;
			if (call->mock == mock &&
				call->funcIndex == funcno)
            funcName = call->funcName;
	    }
        throw ExpectationException(this, &tuple, funcName);
    }
    MockRepository() 
    	: autoExpect(true)
    {
    }
    ~MockRepository() 
    {
		if (!std::uncaught_exception())
			VerifyAll();
		reset();
		for (std::list<base_mock *>::iterator i = mocks.begin(); i != mocks.end(); i++) 
		{
			(*i)->destroy();
		}
		mocks.clear();
    }
	void reset() 
	{
		for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end(); i++) 
		{
			delete *i;
		}
		expectations.clear();
		for (std::list<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end(); i++) 
		{
			delete *i;
		}
		neverCalls.clear();
		for (std::list<Call *>::iterator i = optionals.begin(); i != optionals.end(); i++) 
		{
			delete *i;
		}
		optionals.clear();
	}
    void VerifyAll() 
	{
		for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end(); i++) 
		{
			if (!(*i)->satisfied)
	    		throw CallMissingException(this);
		}
    }
	template <typename base>
	base *InterfaceMock();
	template <typename base>
	base *ClassMock();
};

// mock function providers
template <typename Z, typename Y>
class mockFuncs : public mock<Z> {
private: 
    mockFuncs();
public:
	template <int X>
	Y expectation0()
	{
        MockRepository *repo = mock<Z>::repo;
        return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<>());
	}
	template <int X, typename A>
	Y expectation1(A a)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A>(a));
	}
	template <int X, typename A, typename B>
	Y expectation2(A a, B b)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	Y expectation3(A a, B b, C c)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	Y expectation4(A a, B b, C c, D d)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	Y expectation5(A a, B b, C c, D d, E e)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	Y expectation6(A a, B b, C c, D d, E e, F f)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	Y expectation7(A a, B b, C c, D d, E e, F f, G g)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	Y expectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), new tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	Y expectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	Y expectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	Y expectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	Y expectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	Y expectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	Y expectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
        MockRepository *repo = mock<Z>::repo;
        return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	Y expectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	Y expectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
        MockRepository *repo = mock<Z>::repo;
		return repo->template DoExpectation<Y>(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}
};

template <typename Z>
class mockFuncs<Z, void> : public mock<Z> {
private: 
    mockFuncs();
public:
	template <int X>
	void expectation0()
	{
        MockRepository *repo = mock<Z>::repo;
        repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<>());
	}
	template <int X, typename A>
	void expectation1(A a)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A>(a));
	}
	template <int X, typename A, typename B>
	void expectation2(A a, B b)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B>(a,b));
	}
	template <int X, typename A, typename B, typename C>
	void expectation3(A a, B b, C c)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C>(a,b,c));
	}
	template <int X, typename A, typename B, typename C, typename D>
	void expectation4(A a, B b, C c, D d)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D>(a,b,c,d));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E>
	void expectation5(A a, B b, C c, D d, E e)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E>(a,b,c,d,e));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F>
	void expectation6(A a, B b, C c, D d, E e, F f)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F>(a,b,c,d,e,f));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G>
	void expectation7(A a, B b, C c, D d, E e, F f, G g)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G>(a,b,c,d,e,f,g));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
	void expectation8(A a, B b, C c, D d, E e, F f, G g, H h)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H>(a,b,c,d,e,f,g,h));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I>
	void expectation9(A a, B b, C c, D d, E e, F f, G g, H h, I i)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I>(a,b,c,d,e,f,g,h,i));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J>
	void expectation10(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J>(a,b,c,d,e,f,g,h,i,j));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K>
	void expectation11(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K>(a,b,c,d,e,f,g,h,i,j,k));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L>
	void expectation12(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L>(a,b,c,d,e,f,g,h,i,j,k,l));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M>
	void expectation13(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M>(a,b,c,d,e,f,g,h,i,j,k,l,m));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N>
	void expectation14(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n)
	{
        MockRepository *repo = mock<Z>::repo;
        repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N>(a,b,c,d,e,f,g,h,i,j,k,l,m,n));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O>
	void expectation15(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o));
	}
	template <int X, typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H, typename I, typename J, typename K, typename L, typename M, typename N, typename O, typename P>
	void expectation16(A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p)
	{
        MockRepository *repo = mock<Z>::repo;
		repo->DoVoidExpectation(this, mock<Z>::translateX(X), tuple<A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p));
	}
};

template <typename Z>
void MockRepository::BasicRegisterExpect(mock<Z> *zMock, int funcIndex, void (base_mock::*func)(), int X)
{
	if (zMock->funcMap[funcIndex] == -1)
	{
		zMock->funcs[funcIndex] = getNonvirtualMemberFunctionAddress<void (*)()>(func);
		zMock->funcMap[funcIndex] = X;
	}
}

// Mock repository implementation
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z>
TCall<Y> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)();
	mfp = &mockFuncs<Z2, Y>::template expectation0<X>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex, 
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y> *call = new TCall<Y>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, typename A>
TCall<Y,A> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A);
	mfp = &mockFuncs<Z2, Y>::template expectation1<X,A>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A> *call = new TCall<Y,A>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B>
TCall<Y,A,B> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B);
	mfp = &mockFuncs<Z2, Y>::template expectation2<X,A,B>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B> *call = new TCall<Y,A,B>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C>
TCall<Y,A,B,C> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C);
	mfp = &mockFuncs<Z2, Y>::template expectation3<X,A,B,C>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C> *call = new TCall<Y,A,B,C>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
		break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D>
TCall<Y,A,B,C,D> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D);
	mfp = &mockFuncs<Z2, Y>::template expectation4<X,A,B,C,D>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D> *call = new TCall<Y,A,B,C,D>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E>
TCall<Y,A,B,C,D,E> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E);
	mfp = &mockFuncs<Z2, Y>::template expectation5<X,A,B,C,D,E>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E> *call = new TCall<Y,A,B,C,D,E>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
		break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F>
TCall<Y,A,B,C,D,E,F> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F);
	mfp = &mockFuncs<Z2, Y>::template expectation6<X,A,B,C,D,E,F>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F> *call = new TCall<Y,A,B,C,D,E,F>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
		break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G>
TCall<Y,A,B,C,D,E,F,G> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G);
	mfp = &mockFuncs<Z2, Y>::template expectation7<X,A,B,C,D,E,F,G>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G> *call = new TCall<Y,A,B,C,D,E,F,G>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H>
TCall<Y,A,B,C,D,E,F,G,H> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H);
	mfp = &mockFuncs<Z2, Y>::template expectation8<X,A,B,C,D,E,F,G,H>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H> *call = new TCall<Y,A,B,C,D,E,F,G,H>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I>
TCall<Y,A,B,C,D,E,F,G,H,I> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I);
	mfp = &mockFuncs<Z2, Y>::template expectation9<X,A,B,C,D,E,F,G,H,I>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I> *call = new TCall<Y,A,B,C,D,E,F,G,H,I>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I, typename J>
TCall<Y,A,B,C,D,E,F,G,H,I,J> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J);
	mfp = &mockFuncs<Z2, Y>::template expectation10<X,A,B,C,D,E,F,G,H,I,J>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
		break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K);
	mfp = &mockFuncs<Z2, Y>::template expectation11<X,A,B,C,D,E,F,G,H,I,J,K>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L);
	mfp = &mockFuncs<Z2, Y>::template expectation12<X,A,B,C,D,E,F,G,H,I,J,K,L>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M);
	mfp = &mockFuncs<Z2, Y>::template expectation13<X,A,B,C,D,E,F,G,H,I,J,K,L,M>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N);
	mfp = &mockFuncs<Z2, Y>::template expectation14<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex, 
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}
template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O);
	mfp = &mockFuncs<Z2, Y>::template expectation15<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex, 
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}

template <int X, RegistrationType expect, typename Z2, typename Y, typename Z, 
		  typename A, typename B, typename C, typename D, 
		  typename E, typename F, typename G, typename H,
		  typename I, typename J, typename K, typename L,
		  typename M, typename N, typename O, typename P>
TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> &MockRepository::RegisterExpect_(Z2 *mck, Y (Z::*func)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P), const char *funcName, const char *fileName, unsigned long lineNo) 
{
	int funcIndex = getFunctionIndex(func);
	Y (mockFuncs<Z2, Y>::*mfp)(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P);
	mfp = &mockFuncs<Z2, Y>::template expectation16<X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>;
	BasicRegisterExpect(reinterpret_cast<mock<Z2> *>(mck), 
						funcIndex,
						reinterpret_cast<void (base_mock::*)()>(mfp),X);
	TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P> *call = new TCall<Y,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P>(expect, reinterpret_cast<base_mock *>(mck), funcIndex, lineNo, funcName, fileName);
	switch(expect)
	{
	case Never: neverCalls.push_back(call); break;
	case DontCare: optionals.push_back(call); break;
	case Once:
		if (autoExpect && expectations.size() > 0) 
		{
			call->previousCalls.push_back(expectations.back());
		}
		expectations.push_back(call);
	    break;
	}
	return *call;
}

template <typename Z>
Z MockRepository::DoExpectation(base_mock *mock, int funcno, const base_tuple &tuple) 
{
	for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end(); ++i) 
	{
		Call *call = *i;
		if (call->mock == mock &&
			call->funcIndex == funcno &&
			call->matchesArgs(tuple) &&
			!call->satisfied)
		{
			bool allSatisfy = true;
			for (std::list<Call *>::iterator callsBefore = call->previousCalls.begin();
				callsBefore != call->previousCalls.end(); ++callsBefore)
			{
				if (!(*callsBefore)->satisfied)
				{
					allSatisfy = false;
				}
			}
			if (!allSatisfy) continue;

			call->satisfied = true;

			if (call->eHolder)
				call->eHolder->rethrow();

    		if (call->retVal)
				return ((ReturnValueWrapper<Z> *)call->retVal)->rv;

    		if (call->functor != NULL)
    			return (*(TupleInvocable<Z> *)(call->functor))(tuple);

        throw NoResultSetUpException(call->getArgs(), call->funcName);
		}
	}
	for (std::list<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end(); ++i) 
	{
		Call *call = *i;
		if (call->mock == mock &&
			call->funcIndex == funcno &&
			call->matchesArgs(tuple))
		{
			bool allSatisfy = true;
			for (std::list<Call *>::iterator callsBefore = call->previousCalls.begin();
				callsBefore != call->previousCalls.end(); ++callsBefore)
			{
				if (!(*callsBefore)->satisfied)
				{
					allSatisfy = false;
				}
			}
			if (!allSatisfy) continue;

			call->satisfied = true;

			throw ExpectationException(this, call->getArgs(), call->funcName);
		}
	}
	for (std::list<Call *>::iterator i = optionals.begin(); i != optionals.end(); ++i) 
	{
		Call *call = *i;
		if (call->mock == mock &&
			call->funcIndex == funcno &&
			call->matchesArgs(tuple))
		{
			bool allSatisfy = true;
			for (std::list<Call *>::iterator callsBefore = call->previousCalls.begin();
				callsBefore != call->previousCalls.end(); ++callsBefore)
			{
				if (!(*callsBefore)->satisfied)
				{
					allSatisfy = false;
				}
			}
			if (!allSatisfy) continue;

			call->satisfied = true;
			
			if (call->eHolder)
				call->eHolder->rethrow();

    		if (call->retVal)
				return ((ReturnValueWrapper<Z> *)call->retVal)->rv;
        
        	if (call->functor != NULL)
        		return (*(TupleInvocable<Z> *)(call->functor))(tuple);
        
			throw NoResultSetUpException(call->getArgs(), call->funcName);
		}
	}
	const char *funcName = NULL;
	for (std::list<Call *>::iterator i = expectations.begin(); i != expectations.end() && !funcName; ++i) 
	{
		Call *call = *i;
		if (call->mock == mock &&
			call->funcIndex == funcno)
			funcName = call->funcName;
	}
	for (std::list<Call *>::iterator i = neverCalls.begin(); i != neverCalls.end() && !funcName; ++i) 
    {
	    Call *call = *i;
	    if (call->mock == mock &&
		    call->funcIndex == funcno)
        funcName = call->funcName;
    }
	for (std::list<Call *>::iterator i = optionals.begin(); i != optionals.end() && !funcName; ++i) 
    {
	    Call *call = *i;
	    if (call->mock == mock &&
		    call->funcIndex == funcno)
        funcName = call->funcName;
    }
	throw ExpectationException(this, &tuple, funcName);
}
template <typename base>
base *MockRepository::InterfaceMock() {
	mock<base> *m = new mock<base>(this);
    mocks.push_back(m);
	return reinterpret_cast<base *>(m);
}
template <typename base>
base *MockRepository::ClassMock() {
	classMock<base> *m = new classMock<base>(this);
    mocks.push_back(m);
	return reinterpret_cast<base *>(m);
}

inline std::ostream &operator<<(std::ostream &os, const Call &call)
{
	os << call.fileName << "(" << call.lineno << ") ";
	if (call.expectation == Once)
		os << "Expectation for ";
	else
		os << "Result set for ";

	os << call.funcName;

    if (call.getArgs())
        call.getArgs()->printTo(os);
    else
        os << "(...)";

    os << " on the mock at 0x" << call.mock << " was ";

	if (!call.satisfied)
		os << "not ";

	if (call.expectation == Once)
		os << "satisfied." << std::endl;
	else
		os << "used." << std::endl;

	return os;
}

inline std::ostream &operator<<(std::ostream &os, const MockRepository &repo)
{
	if (repo.expectations.size())
	{
		os << "Expections set:" << std::endl;
		for (std::list<Call *>::const_iterator exp = repo.expectations.begin(); exp != repo.expectations.end(); ++exp)
			os << **exp;
		os << std::endl;
	}

	if (repo.neverCalls.size())
	{
		os << "Functions explicitly expected to not be called:" << std::endl;
		for (std::list<Call *>::const_iterator exp = repo.neverCalls.begin(); exp != repo.neverCalls.end(); ++exp)
			os << **exp;
		os << std::endl;
	}

	if (repo.optionals.size())
	{
		os << "Optional results set up:" << std::endl;
		for (std::list<Call *>::const_iterator exp = repo.optionals.begin(); exp != repo.optionals.end(); ++exp)
			os << **exp;
		os << std::endl;
	}

	return os;
}

inline void BaseException::setException(const char *description, MockRepository *repo) 
{
	std::stringstream text;
	text << description;
	text << *repo;
	std::string result = text.str();
	strncpy(buffer, result.c_str(), sizeof(buffer)-1);
	buffer[sizeof(buffer)-1] = '\0';
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif

