// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * Modified by Michael Wiklund
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SESSION_HPP
#define SESSION_HPP

#include <vector>

#include <Wt/Auth/Login>

#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/backend/Sqlite3>

#include "User.hpp"

typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;


class Session
{
public:
    static void configureAuth();

    Session();
    ~Session();

    Wt::Auth::AbstractUserDatabase&  users();
    Wt::Auth::Login&  login()  {return _login;}

    //// These methods deal with the currently logged in user
    std::string  userName()  const;

    static const Wt::Auth::AuthService&  auth();
    static const Wt::Auth::AbstractPasswordService&  passwordAuth();
    static const std::vector<const Wt::Auth::OAuthService *>&  oAuth();

private:
    Wt::Dbo::backend::Sqlite3  _sqlite3;
    mutable Wt::Dbo::Session  _session;
    UserDatabase*  _users;
    Wt::Auth::Login  _login;

    Wt::Dbo::ptr<User>  user()  const;
};


#endif //SESSION_HPP
