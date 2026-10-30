#if __GNUG__ >= 2
#  pragma implementation
#endif

#include <container/MFSet.h>

//---- newclass ---------------------------------------------------------

template<class Type>
int MFSet<Type>::operator <= (MFSet<Type>& b)
{
	if (count > b.count) return 0;
	if (count == 0) return 1;
	for (Pix i = first(); i; next(i)) if (b.seek((*this)(i)) == 0) return 0;
	return 1;
} 

template<class Type>
int MFSet<Type>::owns(Pix idx)
{
	if (idx == 0) return 0;
	for (Pix i = first(); i; next(i)) 
		if (i == idx) return 1;
	return 0;
}

template<class Type>
Pix MFSet<Type>::seek(Type item) const
{
	extern int EQ(Type&, Type&);
	Pix i;
	for (i = first(); 
				i != 0 && !(EQ((*this)(i), item)); 
				next(i));
	return i;
}        // Pix of item

template<class Type>
void MFSet<Type>::operator |= (MFSet<Type>& b)
{
	if (&b != this)
		for (Pix i = b.first(); i; b.next(i)) add(b(i));
} // add all items in b

template<class Type>
int MFSet<Type>::operator == (MFSet<Type>& b)
{
	int n = count;
	if (n != b.count) return 0;
	if (n == 0) return 1;
	Pix i = first();
	Pix j = b.first();
	while (n-- > 0)
	{
		if ((b.seek((*this)(i)) == 0) || (seek(b(j)) == 0)) return 0;
		next(i);
		b.next(j);
	}
	return 1;
}

template<class Type>
void MFSet<Type>::operator &= (MFSet<Type>& b)
{
	if (&b != this)
	{
		Pix i = first();
		Pix n = i;
		while (i != 0)
		{
			next(n);
			if (b.seek((*this)(i)) == 0) del((*this)(i));
			i = n;
		}
	}
} // delete items not in b

template<class Type>
void MFSet<Type>::operator -= (MFSet<Type>& b)
{
	if (&b == this)
		clear();
	else
		for (Pix i = b.first(); i; b.next(i)) del(b(i));
} // delete items also in b

template<class Type>
void MFSet<Type>::clear()
{
	Pix i = first(); 
	while (i != 0)
	{
		del((*this)(i));
		i = first();
	}
}                // delete all items
