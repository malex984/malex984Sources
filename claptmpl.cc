// emacs edit mode for this file is -*- C++ -*-
/****************************************
*  Computer Algebra System SINGULAR     *
****************************************/
// $Id: claptmpl.cc,v 1.11 1998-01-05 16:39:17 Singular Exp $
/*
* ABSTRACT - instantiation of all templates
*/

#include "mod2.h"

#ifdef HAVE_FACTORY
  #define SINGULAR 1
  #include <factory.h>
#endif

#if defined(HAVE_FACTORY) || defined(HAVE_FGLM)
  #ifdef macintosh
    #include <::templates:ftmpl_list.cc>
  #else
    #include <templates/ftmpl_list.cc>
  #endif

  #ifdef HAVE_FGLM
    #include "fglm.h"
  #endif

  #ifdef HAVE_FACTORY
    #ifdef macintosh
      #include <::templates:ftmpl_array.cc>
      #include <::templates:ftmpl_factor.cc>
      #include <::templates:ftmpl_functions.h>
      #include <::templates:ftmpl_matrix.cc>
      template List<CanonicalForm> Difference<CanonicalForm>(const List<CanonicalForm>&,const List<CanonicalForm>&);
      template List<CanonicalForm> Union<CanonicalForm>(const List<CanonicalForm>&,const List<CanonicalForm>&);
      template List<CFFactor> Union<CFFactor>(const List<CFFactor>&,const List<CFFactor>&);
      template List<Variable> Union<Variable> ( const List<Variable>&, const List<Variable>& );
      template List<Variable> Difference<Variable> ( const List<Variable>&, const List<Variable>& );
    #else
      #include <templates/ftmpl_array.cc>
      #include <templates/ftmpl_factor.cc>
      #include <templates/ftmpl_functions.h>
      #include <templates/ftmpl_matrix.cc>
    #endif

    template class Factor<CanonicalForm>;
    template class List<CFFactor>;
    template class ListItem<CFFactor>;
    template class ListItem<CanonicalForm>;
    template class ListIterator<CFFactor>;
    template class List<CanonicalForm>;
    template class ListIterator<CanonicalForm>;
    template class Array<CanonicalForm>;
    template class List<MapPair>;
    template class ListItem<MapPair>;
    template class ListIterator<MapPair>;
    template class Matrix<CanonicalForm>;
    template class SubMatrix<CanonicalForm>;
    template class Array<REvaluation>;

    #ifndef macintosh
      template List<CFFactor> Union ( const List<CFFactor>&, const List<CFFactor>& );

      template CanonicalForm tmax ( const CanonicalForm&, const CanonicalForm& );
      template CanonicalForm tmin ( const CanonicalForm&, const CanonicalForm& );

      template int tmax ( const int&, const int& );
      template int tmin ( const int&, const int& );
    #endif

  #endif
// place here your own template stuff, not instantiated by factory
  #ifdef HAVE_LIBFAC_P
    #include <factor.h>

// class.h:
template <class T>
class Substitution {
private:
    T _factor;
    T _exp;
public:
    Substitution() : _factor(1), _exp(0) {}
    Substitution( const Substitution<T> & f ) : _factor(f._factor), _exp(f._exp) {}
    Substitution( const T & f, const T & e ) : _factor(f), _exp(e) {}
    Substitution( const T & f ) : _factor(f), _exp(1) {}
    ~Substitution() {}
    Substitution<T>& operator= ( const Substitution<T>& );
    Substitution<T>& operator= ( const T& );
    T factor() const { return _factor; }
    T exp() const { return _exp; }
    friend int operator== ( const Substitution<T>&, const Substitution<T>& );
};

// class.cc
template <class T>
Substitution<T>& Substitution<T>::operator= ( const Substitution<T>& f )
{
    if ( this != &f ) {
	_factor = f._factor;
	_exp = f._exp;
    }
    return *this;
}

template <class T>
Substitution<T>& Substitution<T>::operator= ( const T & f )
{
    _factor = f;
    _exp = 1;
    return *this;
}

template <class T>
int operator== ( const Substitution<T> &f1, const Substitution<T> &f2 )
{
    return (f1.exp() == f2.exp()) && (f1.factor() == f2.factor());
}

    template class List<int>;
    template class ListIterator<int>;

    template class List<IntList>;
    template class ListIterator<IntList>;

    template class Substitution<CanonicalForm>;
    template class Array<Variable>;
    template class Array<int>;
    typedef Substitution<CanonicalForm> SForm ;
    template class List<SForm>;
    template class ListIterator<SForm>;
    template class List<Variable>;
    template class ListIterator<Variable> ;

    #ifndef macintosh
      template List<Variable> Union ( const List<Variable>&, const List<Variable>& );
      template List<Variable> Difference ( const List<Variable>&, const List<Variable>& );

      template List<CanonicalForm> Union ( const List<CanonicalForm>&, const List<CanonicalForm>& );
      template List<CanonicalForm> Difference ( const List<CanonicalForm>&, const List<CanonicalForm>& );
    #endif

// for charsets:
    template class List<CFList>;
    template class ListIterator<CFList>;

  #endif
#endif

#ifdef HAVE_FGLM
// templates for fglm:

  template class List<fglmSelem>;
  template class ListIterator<fglmSelem>;

  template class List<fglmDelem>;
  template class ListIterator<fglmDelem>;
#endif
