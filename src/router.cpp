#include <mod/router.hpp>

#include <fmt/format.h>

int Router::operator() (const Pos& origin, const Pos& destination)
{
    return ++count;
}
