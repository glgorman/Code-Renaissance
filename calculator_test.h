
#define PI (3.14159265358979323)
typedef enum { add=0, sub=1, mult=2, divi=3, }  opcode;
typedef double (*ftable[])(const double&, const double&);

class calculator
{
protected:
	double reg[16];
	static double (*ftab[4])(const double&, const double&);
	static double fadd(const double &, const double &);
	static double fsub(const double &, const double &);
	static double fmult(const double &, const double &);
	static double fdiv(const double &, const double &);

public:
	static double exec(opcode, const double &arg1, const double &arag2);
	static int main();
};

class real
{
protected:
	union
	{
		int		i;
		DWORD	dw;
		struct
		{
			unsigned int  f:23;
			unsigned int  e:8;
			unsigned int  s:1;
		};
	};
	short	sign();
	short	exp();
	int		frac();

	static real add(real,real);
	static real sub(real,real);

public:
	bool operator ==(real arg);
	bool operator !=(real arg);
	bool operator >=(real arg);
	bool operator <=(real arg);
	bool operator >(real arg);
	bool operator <(real arg);
	real operator +(real arg);
	real operator +=(real arg);
	real operator -(real arg);
	real operator -=(real arg);
	real operator *(real arg);
	real operator /(real arg);
	real operator =(real arg);
	real () { dw = 0; }
	real (float);
	operator float ();
//	operator silly_rabbit ();
};