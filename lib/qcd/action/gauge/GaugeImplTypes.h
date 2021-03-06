/*************************************************************************************

Grid physics library, www.github.com/paboyle/Grid

Source file: ./lib/qcd/action/gauge/GaugeImpl.h

Copyright (C) 2015

Author: paboyle <paboyle@ph.ed.ac.uk>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

See the full license in the file "LICENSE" in the top level distribution
directory
*************************************************************************************/
/*  END LEGAL */
#ifndef GRID_GAUGE_IMPL_TYPES_H
#define GRID_GAUGE_IMPL_TYPES_H

namespace Grid {
namespace QCD {

////////////////////////////////////////////////////////////////////////
// Implementation dependent gauge types
////////////////////////////////////////////////////////////////////////

#define INHERIT_GIMPL_TYPES(GImpl)                  \
  typedef typename GImpl::Simd Simd;                \
  typedef typename GImpl::LinkField GaugeLinkField; \
  typedef typename GImpl::Field GaugeField;         \
  typedef typename GImpl::SiteField SiteGaugeField; \
  typedef typename GImpl::SiteLink SiteGaugeLink;

#define INHERIT_FIELD_TYPES(Impl)             \
  typedef typename Impl::Simd Simd;           \
  typedef typename Impl::SiteField SiteField; \
  typedef typename Impl::Field Field;

// hardcodes the exponential approximation in the template
template <class S, int Nrepresentation = Nc, int Nexp = 12 > class GaugeImplTypes {
public:
  typedef S Simd;

  template <typename vtype> using iImplGaugeLink  = iScalar<iScalar<iMatrix<vtype, Nrepresentation>>>;
  template <typename vtype> using iImplGaugeField = iVector<iScalar<iMatrix<vtype, Nrepresentation>>, Nd>;

  typedef iImplGaugeLink<Simd>  SiteLink;
  typedef iImplGaugeField<Simd> SiteField;

  typedef Lattice<SiteLink>  LinkField; 
  typedef Lattice<SiteField> Field;

  // Guido: we can probably separate the types from the HMC functions
  // this will create 2 kind of implementations
  // probably confusing the users
  // Now keeping only one class


  // Move this elsewhere? FIXME
  static inline void AddLink(Field &U, LinkField &W,
                                  int mu) { // U[mu] += W
    PARALLEL_FOR_LOOP
    for (auto ss = 0; ss < U._grid->oSites(); ss++) {
      U._odata[ss]._internal[mu] =
          U._odata[ss]._internal[mu] + W._odata[ss]._internal;
    }
  }

  ///////////////////////////////////////////////////////////
  // Move these to another class
  // HMC auxiliary functions 
  static inline void generate_momenta(Field &P, GridParallelRNG &pRNG) {
    // specific for SU gauge fields
    LinkField Pmu(P._grid);
    Pmu = zero;
    for (int mu = 0; mu < Nd; mu++) {
      SU<Nrepresentation>::GaussianFundamentalLieAlgebraMatrix(pRNG, Pmu);
      PokeIndex<LorentzIndex>(P, Pmu, mu);
    }
  }

  static inline Field projectForce(Field &P) { return Ta(P); }
  
  static inline void update_field(Field& P, Field& U, double ep){
    for (int mu = 0; mu < Nd; mu++) {
      auto Umu = PeekIndex<LorentzIndex>(U, mu);
      auto Pmu = PeekIndex<LorentzIndex>(P, mu);
      Umu = expMat(Pmu, ep, Nexp) * Umu;
      PokeIndex<LorentzIndex>(U, ProjectOnGroup(Umu), mu);
    }
  }

  static inline RealD FieldSquareNorm(Field& U){
    LatticeComplex Hloc(U._grid);
    Hloc = zero;
    for (int mu = 0; mu < Nd; mu++) {
      auto Umu = PeekIndex<LorentzIndex>(U, mu);
      Hloc += trace(Umu * Umu);
    }
    Complex Hsum = sum(Hloc);
    return Hsum.real();
  }

  static inline void HotConfiguration(GridParallelRNG &pRNG, Field &U) {
    SU<Nc>::HotConfiguration(pRNG, U);
  }

  static inline void TepidConfiguration(GridParallelRNG &pRNG, Field &U) {
    SU<Nc>::TepidConfiguration(pRNG, U);
  }

  static inline void ColdConfiguration(GridParallelRNG &pRNG, Field &U) {
    SU<Nc>::ColdConfiguration(pRNG, U);
  }
};


typedef GaugeImplTypes<vComplex, Nc> GimplTypesR;
typedef GaugeImplTypes<vComplexF, Nc> GimplTypesF;
typedef GaugeImplTypes<vComplexD, Nc> GimplTypesD;

typedef GaugeImplTypes<vComplex, SU<Nc>::AdjointDimension> GimplAdjointTypesR;
typedef GaugeImplTypes<vComplexF, SU<Nc>::AdjointDimension> GimplAdjointTypesF;
typedef GaugeImplTypes<vComplexD, SU<Nc>::AdjointDimension> GimplAdjointTypesD;


} // QCD
} // Grid

#endif // GRID_GAUGE_IMPL_TYPES_H
