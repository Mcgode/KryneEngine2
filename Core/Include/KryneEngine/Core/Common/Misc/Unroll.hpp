/**
 * @file
 * @author Max Godefroy
 * @date 31/03/2025.
 */

#pragma once

#include <cstddef>
#include <EABase/config/eacompilertraits.h>

// See https://gcc.godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGEgMykrgAyeAyYAHI%2BAEaYxCD%2BAKykAA6oCoRODB7evgGp6ZkCoeFRLLHxSbaY9o4CQgRMxAQ5Pn5cgXaYDlkNTQQlkTFxCckKjc2teR22EwNhQ%2BUjSQCUtqhexMjsHOb%2BYcjeWADUJv5uyOPoWFRn2CYaAIIPjwSYLCkGb6fnGQBemAA%2BgRjgAVVCkY7/IEggCSDCwqh%2BABFjhpIYcmAoFMcAGJeBimfz3J7ETAARy8eDJOIg4J%2BbjOqPhiJWLzCtAWx0BgKYBAIxDw0S8bx5UDEAHcmABPBSAjkLFYrY4AN1QeHQxzkDGInloEHxhPMADYTWyniYAOxWS3Il4vN4fL6YBnQ4FgiFQvAA90szBIplojEGbF4glEkmPMmU6mYWn0u7HP2qc2PBXhbm8/mC4WiwHi2hS2Xyhic8JK1XqzXa3W0fWG0xmU1N7lUcOpq1WJ7HHut8MQZNs/xdx69rU6vVnNzgyHJ06WY5cO4QQFtwlDkdWu0Wi1mfaEo4uqdOcZk1h3e17g6HhmXAjoTnRC87tOCY4sJhhCDKzsvMcKYhDHQCBHDYCANCVM5N27Xt0A2aJ6GOYgUTRKC/17GtJ3OSRlxMRIrCbPDkQgJgRVQbkf2tU4YLHHtkOsQMuGOAB6JCgO/KDqNHXstw3e0aKhe8QBQDYQSnKckLQgSyQITYGFQ4d7VtfjXxBD8wlxEhv1Oa10J7AD2NAzBwMgxSXzHOCvAQl16P8VENCk7ie34ZCSLI448BQhzhw8hljhwny8AYyxKIsLjaN7ejLEYli2IRDifL0nTt2eASrmEtARQZCTiEcscZLkhTN2Ul8wjUz8GAAWUMLwxEwuttKUkd/0A%2BKjJMvjzNg%2BDENs%2By8si%2BcrDsxdYta4CN3CwaGJGpjWPGhKwqSqLhtROa4omzjlqGmL5vYyaVP/ISRKy8TzkksynKQzBZOIeTvOKlKODWWhOESXg/A4LRSFQThGWihcFA2LYjz3HhSAITRnrWABrBJ/AAOn8ZGUdR1HjX0ThJA%2BqGfs4XgFBAdFIa%2B57SDgWAkDQD46DichKGplJafiJoWBVS0AFpMSMLgAE4uHRGhaDeYhCYgaJceiMImmlThwal5hiGlAB5aJtG6EnwepthBGV0tZdJ0gsGFYA3DEWhCe4XgsA/IxxEN/AyR6FU41x/1uhFHZwbKmpccfQClY8LBcYFPAWDl3gXeIaJ0kwZF3kMYAy1AUm1ioAxgAUAA1PBMAlZWUkYCOZEEEQxHYKQS/kJQ1Fx3QuH0ROUGCmxH0JyA1lQFI6gYS2OeV4Bjg5q4mUbAiLDMb7UCjwUsHb79qlqLIXARKY/D3IIEUGMoKhARJeYKDIe7XkAN7SI%2Bsm34Z4n3xeNd6OYT43roenqOYr6WG%2BD/Gfon86d%2BFg7xGPvNYQNNjbAkC9N6ONDa/Q4McVm7MOZMGONzQefMEZcARhoY4EBcCEBIPOfwXAVi8BJloJUpA4bIyRmjOhyMMavQ4NjUgn0p741sETCGUM1gU0QCAUSKQRT0wgIzZmERWA7FUAADmNBzY0khUEGCMMcRGkhMG8EwPgIgs89D8FLqIcQld9HVxUOoQ29dSASkAikCOUCODvVYbjOBysRRCJBKgKgCDiBs05igtBi5eaYOwbgjwNN6BRWIaQ7hqdYbw1ofQtGmNmEwPYRwAmXDyHQ2SWYVJvA4FkJ4WsKOGRnCSCAA
// for insight regarding the loop unroll optimization.

namespace KryneEngine
{
    template <size_t To, size_t Index = 0, class Func>
        requires (To <= Index)
    EA_FORCE_INLINE void Unroll(Func&&)
    {}

    template <size_t To, size_t Index = 0, class Func>
    requires (To > Index)
    EA_FORCE_INLINE void Unroll(Func&& _func)
    {
        _func(Index);
        Unroll<To, Index + 1>(_func);
    }

#if defined(__OPTIMIZE__)
    // Will be inlined by optimize pass.
#   define UNROLL_FOR_LOOP(IndexVar, To) Unroll<To>([&](auto IndexVar) {
#   define END_UNROLL() });
#else
    // If there is no optimization, cancel the unroll, as the template unroll function call won't be inlined and would
    // add more overhead than a classic for loop.
#   define UNROLL_FOR_LOOP(IndexVar, To) for (auto IndexVar = 0u; IndexVar < To; ++IndexVar) {
#   define END_UNROLL() }
#endif
}