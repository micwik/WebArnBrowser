/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * Modified by Michael Wiklund
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.hpp"

#include "Wt/Auth/AuthService"
#include "Wt/Auth/HashFunction"
#include "Wt/Auth/PasswordService"
#include "Wt/Auth/PasswordStrengthValidator"
#include "Wt/Auth/PasswordVerifier"
#include "Wt/Auth/GoogleService"
#include "Wt/Auth/Dbo/AuthInfo"
#include "Wt/Auth/Dbo/UserDatabase"

#include <Wt/WApplication>
#include <Wt/WLogger>

#ifndef WIN32
#include <unistd.h>
#endif

#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(ANDROID)
#define HAVE_CRYPT
#endif

using namespace Wt;
namespace dbo = Wt::Dbo;


namespace {

class MyOAuth : public std::vector<const Auth::OAuthService *>
{
public:
    ~MyOAuth()
    {
        for (unsigned i = 0; i < size(); ++i)
            delete (*this)[i];
    }
};

    Auth::AuthService  myAuthService;
    Auth::PasswordService  myPasswordService( myAuthService);
    MyOAuth  myOAuthServices;
}


void  Session::configureAuth()
{
  // myAuthService.setAuthTokensEnabled(true, "XXXcookie");
  // myAuthService.setEmailVerificationEnabled(true);

    Auth::PasswordVerifier*  verifier = new Auth::PasswordVerifier();
    verifier->addHashFunction(new Auth::BCryptHashFunction(7));

    myPasswordService.setVerifier( verifier);
    myPasswordService.setStrengthValidator(new Auth::PasswordStrengthValidator());
    myPasswordService.setAttemptThrottlingEnabled(true);

    if (Auth::GoogleService::configured())
        myOAuthServices.push_back(new Auth::GoogleService( myAuthService));
}


Session::Session()
  : _sqlite3(WApplication::instance()->appRoot() + "user.db")
{
    _session.setConnection( _sqlite3);
    _sqlite3.setProperty("show-queries", "true");

    _session.mapClass<User>("user");
    _session.mapClass<AuthInfo>("auth_info");
    _session.mapClass<AuthInfo::AuthIdentityType>("auth_identity");
    _session.mapClass<AuthInfo::AuthTokenType>("auth_token");

    _users = new UserDatabase( _session);

    dbo::Transaction transaction( _session);
    try {
        _session.createTables();

        //// Add a default guest/guest account
        Auth::User guestUser = _users->registerNew();

        guestUser.addIdentity( Auth::Identity::LoginName, "nisse");
        myPasswordService.updatePassword( guestUser, "tomte1224");

        // guestUser.addIdentity( Auth::Identity::LoginName, "n");
        // myPasswordService.updatePassword( guestUser, "t");

        Wt::log("info") << "Database created";
    }
    catch (...) {
        Wt::log("info") << "Using existing database";
    }

    transaction.commit();
}


Session::~Session()
{
    delete _users;
}


dbo::ptr<User>  Session::user()  const
{
    if (_login.loggedIn()) {
        dbo::ptr<AuthInfo> authInfo = _users->find( _login.user());
        dbo::ptr<User> user = authInfo->user();

        if (!user) {
            user = _session.add(new User());
            authInfo.modify()->setUser(user);
        }

        return user;
    }
    else
        return dbo::ptr<User>();
}


std::string  Session::userName()  const
{
    if (_login.loggedIn())
        return _login.user().identity( Auth::Identity::LoginName).toUTF8();
    else
        return std::string();
}


Auth::AbstractUserDatabase&  Session::users()
{
    return *_users;
}


const Auth::AuthService&  Session::auth()
{
    return myAuthService;
}


const Auth::AbstractPasswordService&  Session::passwordAuth()
{
    return myPasswordService;
}


const std::vector<const Auth::OAuthService *>&  Session::oAuth()
{
    return myOAuthServices;
}
