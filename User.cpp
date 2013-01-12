/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * Modified by Michael Wiklund
 *
 * See the LICENSE file for terms of use.
 */

#include "User.hpp"

#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Dbo/Impl>

DBO_INSTANTIATE_TEMPLATES(User);

using namespace Wt;
using namespace Wt::Dbo;

User::User()
    // : xxxUserValue(0)
{
}
