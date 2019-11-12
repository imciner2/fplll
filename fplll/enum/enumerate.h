/* Copyright (C) 2008-2011 Xavier Pujol
   (C) 2015 Michael Walter.
   (C) 2016 Marc Stevens. (generic improvements, auxiliary solutions, subsolutions)
   (C) 2016 Guillaume Bonnoron. (CVP improvements)

   This file is part of fplll. fplll is free software: you
   can redistribute it and/or modify it under the terms of the GNU Lesser
   General Public License as published by the Free Software Foundation,
   either version 2.1 of the License, or (at your option) any later version.

   fplll is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with fplll. If not, see <http://www.gnu.org/licenses/>. */

#ifndef FPLLL_ENUMERATE_H
#define FPLLL_ENUMERATE_H

#include <array>
#include <fplll/enum/enumerate_base.h>
#include <fplll/enum/enumerate_dyn.h>
#include <fplll/enum/enumerate_ext.h>
#include <fplll/enum/evaluator.h>
#include <fplll/enum/parallel-enum.h>
#include <fplll/gso_interface.h>
#include <memory>

FPLLL_BEGIN_NAMESPACE

template <typename ZT, typename FT> class Enumeration
{
public:
  Enumeration(MatGSOInterface<ZT, FT> &gso, Evaluator<FT> &evaluator,
              const vector<int> &max_indices = vector<int>())
      : _gso(gso), _evaluator(evaluator), _max_indices(max_indices), enumdyn(nullptr), _nodes(0)
  {
  }

  void enumerate(int first, int last, FT &fmaxdist, long fmaxdistexpo,
                 const vector<FT> &target_coord = vector<FT>(),
                 const vector<enumxt> &subtree  = vector<enumxt>(),
                 const vector<enumf> &pruning = vector<enumf>(), bool dual = false,
                 bool subtree_reset = false)
  {
    // check for external enumerator and use that
    if (get_external_enumerator() != nullptr && subtree.empty() && target_coord.empty())
    {
      if (enumext.get() == nullptr)
        enumext.reset(new ExternalEnumeration<ZT, FT>(_gso, _evaluator));
      if (enumext->enumerate(first, last, fmaxdist, fmaxdistexpo, pruning, dual))
      {
        _nodes = enumext->get_nodes();
        return;
      }
    }
    // if external enumerator is not available, not possible or when it fails then fall through to
    // fplll enumeration
    if (get_threads() > 1 && last - first >= 40 && dual == false && subtree_reset == false &&
        _max_indices.empty())
    {
      if (enumparalleldyn.get() == nullptr)
        enumparalleldyn.reset(new ParallelEnumerationDyn<ZT, FT>(_gso, _evaluator));
      enumparalleldyn->enumerate(first, last, fmaxdist, fmaxdistexpo, -1, target_coord, subtree,
                                 pruning);
      _nodes = enumparalleldyn->get_nodes();
    }
    else
    {
      if (enumdyn.get() == nullptr)
        enumdyn.reset(new EnumerationDyn<ZT, FT>(_gso, _evaluator, _max_indices));
      enumdyn->enumerate(first, last, fmaxdist, fmaxdistexpo, target_coord, subtree, pruning, dual,
                         subtree_reset);
      _nodes = enumdyn->get_nodes();
    }
  }

  inline uint64_t get_nodes() const { return _nodes; }

private:
  MatGSOInterface<ZT, FT> &_gso;
  Evaluator<FT> &_evaluator;
  vector<int> _max_indices;
  std::unique_ptr<EnumerationDyn<ZT, FT>> enumdyn;
  std::unique_ptr<ParallelEnumerationDyn<ZT, FT>> enumparalleldyn;
  std::unique_ptr<ExternalEnumeration<ZT, FT>> enumext;
  uint64_t _nodes;
};

FPLLL_END_NAMESPACE

#endif
