#ifndef _VEILER_FORSETI_OLYMPUS_HPP_INCLUDED
#define _VEILER_FORSETI_OLYMPUS_HPP_INCLUDED

#include<fstream>

namespace veiler{

namespace forseti{

#define _IOS(A) std::ios::A
#define _OM _IOS(openmode)
#define _OMC static_cast<_OM>
#define _OMT static const _OM
_OMT  r   = _OMC(_IOS(in)                                               );
_OMT  w   = _OMC(_IOS(out)                                              );
_OMT  a   = _OMC(_IOS(out) | _IOS(app)                                  );
_OMT  rb  = _OMC(_IOS(in)  | _IOS(binary)                               );
_OMT  wb  = _OMC(_IOS(out) | _IOS(binary)                               );
_OMT  ab  = _OMC(_IOS(out) | _IOS(binary) | _IOS(app)                   );
_OMT  rp  = _OMC(_IOS(in)  | _IOS(out)                                  );
_OMT  wp  = _OMC(_IOS(in)  | _IOS(out)    | _IOS(trunc)                 );
_OMT  ap  = _OMC(_IOS(in)  | _IOS(out)    | _IOS(app)                   );
_OMT  rpb = _OMC(_IOS(in)  | _IOS(out)    | _IOS(binary)                );
_OMT  wpb = _OMC(_IOS(in)  | _IOS(out)    | _IOS(trunc)  | _IOS(binary) );
_OMT  apb = _OMC(_IOS(in)  | _IOS(out)    | _IOS(app)    | _IOS(binary) );
_OMT& rbp = rpb                                                          ;
_OMT& wbp = wpb                                                          ;
_OMT& abp = apb                                                          ;
#undef _OMT
#undef _OMC
#undef _OM
#undef _IOS

}//End : namespace forseti

}//End : namespace veiler

#endif //_VEILER_FORSETI_OLYMPUS_HPP_INCLUDED

//Copyright (C) 2012 I
//  Distributed under the Veiler Source License 1.0.
