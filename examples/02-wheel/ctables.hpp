#include "asciirast/math/types.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace math = asciirast::math;

using ctable = std::array<std::array<char, 3>, 3>;

using FramebufferPoint = std::tuple<math::Vec2Int, std::tuple<char, math::Vec3>>;

static constexpr char ign = '\0'; // ignore

//  possible chars: ' ', '\\', '|', '/', '_'

// static constexpr ctable ctable000 = { { { '_', ' ', '_' },     // _ _
//                                         { ' ', 'V', ' ' },     //  V
//                                         { ign, ign, ign } } }; //

static constexpr ctable ctable001 = { { { ign, '_', ign },     //  _
                                        { '_', '|', ign },     // _|
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable002 = { { { ign, '_', ign },     //  _
                                        { ign, '|', '_' },     //  |_
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable003 = { { { ign, ign, ign },     //
                                        { '_', '_', '_' },     // ___
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable004 = { { { ign, ign, '_' },     //   _
                                        { '_', '/', ign },     // _/
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable005 = { { { '_', ign, ign },     // _
                                        { ign, '\\', '_' },    /*  \_ */
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable006 = { { { ign, '|', ign },     //  |
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable007 = { { { ign, '/', ign },     //  /
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable008 = { { { ign, '|', ign },      //  |
                                        { ign, '|', ign },      //  |
                                        { ign, '\\', ign } } }; /*  \ */

static constexpr ctable ctable009 = { { { ign, '/', ign },      //  /
                                        { ign, '|', ign },      //  |
                                        { ign, '\\', ign } } }; /*  \ */

static constexpr ctable ctable010 = { { { '\\', ign, ign },    /* \  */
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable011 = { { { '\\', ign, ign },    /* \  */
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable012 = { { { '\\', ign, ign },     /* \  */
                                        { ign, '\\', ign },     /*  \ */
                                        { ign, ign, '\\' } } }; /*   \ */

static constexpr ctable ctable013 = { { { ign, ign, '/' },      //   /
                                        { ign, '|', ign },      //  |
                                        { ign, ign, '\\' } } }; /*   \ */

static constexpr ctable ctable014 = { { { ign, ign, '/' },     //   /
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; //  |

static constexpr ctable ctable015 = { { { ign, ign, '/' },     /*   /  */
                                        { ign, '/', ign },     /*  /   */
                                        { '/', ign, ign } } }; /* /    */

static constexpr ctable ctable016 = { { { ign, '|', ign },     //  |
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable017 = { { { ign, '|', ign },      /*  |   */
                                        { ign, '|', ign },      /*  |   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable018 = { { { '|', ign, ign },      /* |    */
                                        { ign, '\\', ign },     /*  \   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable019 = { { { ign, ign, '|' },     /*   |   */
                                        { ign, '/', ign },     /*  /    */
                                        { '/', ign, ign } } }; /* /     */

static constexpr ctable ctable020 = { { { '_', ign, ign },      /* _    */
                                        { ign, '\\', ign },     /*  \   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable021 = { { { ign, ign, '_' },     /*   _   */
                                        { ign, '/', ign },     /*  /    */
                                        { '/', ign, ign } } }; /* /     */

static constexpr ctable ctable022 = { { { '\\', ign, ign },    /* \     */
                                        { ign, '\\', '_' },    /*  \_   */
                                        { ign, ign, ign } } }; /*       */

static constexpr ctable ctable023 = { { { ign, ign, '/' },     /*   /   */
                                        { '_', '/', ign },     /* _/    */
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable024 = { { { ign, '\\', ign },    /*  \  */
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable025 = { { { ign, '\\', ign },     /*  \   */
                                        { ign, '|', ign },      /*  |   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable026 = { { { ign, '/', ign },     /*  /  */
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; // /

static constexpr ctable ctable027 = { { { ign, '/', ign },      /*  /   */
                                        { ign, '|', ign },      /*  |   */
                                        { ign, ign, '\\' } } }; /*   \  */

static constexpr ctable ctable028 = { { { '|', ign, ign },     /* |   */
                                        { ign, '\\', ign },    /*  \  */
                                        { ign, ign, '|' } } }; //   |

static constexpr ctable ctable029 = { { { ign, ign, '|' },     /*   |   */
                                        { ign, '/', ign },     /*  /    */
                                        { '|', ign, ign } } }; /* |     */

static constexpr ctable ctable030 = { { { '|', ign, ign },     /* |   */
                                        { ign, '\\', '_' },    /*  \_ */
                                        { ign, ign, ign } } }; //

static constexpr ctable ctable031 = { { { ign, ign, '|' },     /*   |   */
                                        { '_', '/', ign },     /* _/    */
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable032 = { { { ign, ign, '|' },      //   |
                                        { ign, '|', ign },      //  |
                                        { ign, ign, '\\' } } }; /*   \ */

static constexpr ctable ctable033 = { { { ign, ign, '/' },     //   /
                                        { ign, '|', ign },     //  |
                                        { ign, ign, '|' } } }; /*   | */

static constexpr ctable ctable034 = { { { '|', ign, ign },     // |
                                        { ign, '|', ign },     //  |
                                        { '/', ign, ign } } }; /* /   */

static constexpr ctable ctable035 = { { { '|', ign, ign },     /* |   */
                                        { ign, '|', ign },     //  |
                                        { '|', ign, ign } } }; /* |   */

static constexpr ctable ctable036 = { { { '|', ign, ign },     /* |   */
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; /*  |   */

static constexpr ctable ctable037 = { { { ign, ign, '|' },     /*   | */
                                        { ign, '|', ign },     //  |
                                        { ign, '|', ign } } }; /*  |   */

static constexpr ctable ctable038 = { { { ign, '|', ign },     /*   | */
                                        { '_', '|', ign },     //  _|
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable039 = { { { ign, '/', ign },     /*   / */
                                        { '_', '|', ign },     //  _|
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable040 = { { { ign, '\\', ign },    /*   \ */
                                        { '_', '|', ign },     //  _|
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable041 = { { { ign, '|', ign },     /*   | */
                                        { ign, '|', '_' },     //   |_
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable042 = { { { ign, '/', ign },     /*   / */
                                        { ign, '|', '_' },     //   |_
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable043 = { { { ign, '\\', ign },    /*   \ */
                                        { ign, '|', '_' },     //   |_
                                        { ign, ign, ign } } }; /*      */

static constexpr ctable ctable044 = { { { '_', ign, ign },     /* _     */
                                        { ign, '\\', ign },    /*  \    */
                                        { ign, '|', ign } } }; /*  |    */

static constexpr ctable ctable045 = { { { ign, ign, '_' },     /*   _   */
                                        { ign, '/', ign },     /*  /    */
                                        { ign, '|', ign } } }; /*  |    */

static constexpr auto ctables = std::to_array<ctable>(
        { ctable001, ctable002, ctable003, ctable004, ctable005, ctable006, ctable007, ctable008, ctable009,
          ctable010, ctable011, ctable012, ctable013, ctable014, ctable015, ctable016, ctable017, ctable018,
          ctable019, ctable020, ctable021, ctable022, ctable023, ctable024, ctable025, ctable026, ctable027,
          ctable028, ctable029, ctable030, ctable031, ctable032, ctable033, ctable034, ctable035, ctable036,
          ctable037, ctable038, ctable039, ctable040, ctable041, ctable042, ctable043, ctable044, ctable045 });
