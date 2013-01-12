// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * Modified by Michael Wiklund
 *
 * See the LICENSE file for terms of use.
 */

#ifndef USER_HPP
#define USER_HPP

#include <Wt/WDateTime>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Auth/Dbo/AuthInfo>

#include <string>

class User;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
typedef Wt::Dbo::collection< Wt::Dbo::ptr<User> > Users;


class User : public Wt::Dbo::Dbo<User>
{
public:
    User();

    std::string name;  // a copy of auth info's user name

    // int xxxUserValue;
    Wt::Dbo::collection< Wt::Dbo::ptr<AuthInfo> >  authInfos;

    template<class Action>
    void  persist( Action& a)
    {
        // Wt::Dbo::field(a, xxxUserValue, "xxxUserValue");

        Wt::Dbo::hasMany(a, authInfos, Wt::Dbo::ManyToOne, "user");
    }
};

DBO_EXTERN_TEMPLATES(User);


#endif // USER_HPP
