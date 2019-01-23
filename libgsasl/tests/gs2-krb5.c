/* gs2-krb5.c --- Test the GS2-KRB5 mechanism.
 * Copyright (C) 2002-2012 Simon Josefsson
 *
 * This file is part of GNU SASL.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utils.h"

#define SERVICE "host"
#define HOST "latte.josefsson.org"
#define GSSAPI_USER "jas"

static const char *AUTHZID[] = {
  "foo", "BAB,ABA", ",=,=", "=", ""
};

size_t i;

static int
callback (Gsasl * ctx, Gsasl_session * sctx, Gsasl_property prop)
{
  int rc = GSASL_NO_CALLBACK;

  switch (prop)
    {
    case GSASL_AUTHZID:
      if (*AUTHZID[i])
	{
	  gsasl_property_set (sctx, GSASL_AUTHZID, AUTHZID[i]);
	  rc = GSASL_OK;
	}
      break;

    case GSASL_SERVICE:
      gsasl_property_set (sctx, prop, SERVICE);
      rc = GSASL_OK;
      break;

    case GSASL_HOSTNAME:
      gsasl_property_set (sctx, prop, HOST);
      rc = GSASL_OK;
      break;

    case GSASL_VALIDATE_GSSAPI:
      {
	const char *client_name =
	  gsasl_property_fast (sctx, GSASL_GSSAPI_DISPLAY_NAME);
	const char *authzid = gsasl_property_fast (sctx, GSASL_AUTHZID);

	if (client_name)
	  printf ("GSSAPI user: %s\n", client_name);
	else
	  fail ("no client name\n");
	if (authzid)
	  printf ("Authorization ID: %s\n", authzid);

	if (client_name && strcmp (client_name, GSSAPI_USER) == 0 &&
	    ((authzid == NULL && *AUTHZID[i] == '\0')
	     || strcmp (authzid, AUTHZID[i]) == 0))
	  rc = GSASL_OK;
	else
	  rc = GSASL_AUTHENTICATION_ERROR;
      }
      break;

    default:
      fail ("Unknown callback property %d\n", prop);
      break;
    }

  return rc;
}

static char
ret_char (int rc)
{
  if (rc == GSASL_OK)
    return 'O';
  else if (rc == GSASL_NEEDS_MORE)
    return 'N';
  else
    return '?';
}

void
doit (void)
{
  Gsasl *ctx = NULL;
  Gsasl_session *server = NULL, *client = NULL;
  char *s1 = NULL, *s2 = NULL;
  int rc, res1, res2;

  if (getenv ("GNUGSS") && strcmp (getenv ("GNUGSS"), "no") == 0)
    {
      fail ("Not using GNU GSS, skipping self-test.\n");
      exit (77);
    }

  rc = gsasl_init (&ctx);
  if (rc != GSASL_OK)
    {
      fail ("gsasl_init() failed (%d):\n%s\n", rc, gsasl_strerror (rc));
      return;
    }

  if (!gsasl_client_support_p (ctx, "GS2-KRB5")
      || !gsasl_server_support_p (ctx, "GS2-KRB5"))
    {
      gsasl_done (ctx);
      fail ("No support for GS2-KRB5.\n");
      exit (77);
    }

  gsasl_callback_set (ctx, callback);

  for (i = 0; i < 5; i++)
    {
      bool client_first = (i % 2) == 0;

      rc = gsasl_server_start (ctx, "GS2-KRB5", &server);
      if (rc != GSASL_OK)
	{
	  fail ("gsasl_server_start() failed (%d):\n%s\n",
		rc, gsasl_strerror (rc));
	  return;
	}
      rc = gsasl_client_start (ctx, "GS2-KRB5", &client);
      if (rc != GSASL_OK)
	{
	  fail ("gsasl_client_start() failed (%d):\n%s\n",
		rc, gsasl_strerror (rc));
	  return;
	}

      if (client_first)
	{
	  rc = gsasl_step64 (client, NULL, &s1);
	  if (rc != GSASL_OK && rc != GSASL_NEEDS_MORE)
	    {
	      fail ("gsasl_step64 failed (%d):\n%s\n", rc,
		    gsasl_strerror (rc));
	      return;
	    }

	  if (debug)
	    printf ("C: %s [%c]\n", s1, ret_char (rc));
	}

      do
	{
	  res1 = gsasl_step64 (server, s1, &s2);
	  if (s1 == NULL && res1 == GSASL_OK)
	    fail ("gsasl_step64 direct success?\n");
	  if (s1)
	    {
	      gsasl_free (s1);
	      s1 = NULL;
	    }
	  if (res1 != GSASL_OK && res1 != GSASL_NEEDS_MORE)
	    {
	      fail ("gsasl_step64 (1) failed (%d):\n%s\n", res1,
		    gsasl_strerror (res1));
	      return;
	    }

	  if (debug)
	    printf ("S: %s [%c]\n", s2, ret_char (res1));

	  res2 = gsasl_step64 (client, s2, &s1);
	  gsasl_free (s2);
	  if (res2 != GSASL_OK && res2 != GSASL_NEEDS_MORE)
	    {
	      fail ("gsasl_step64 (2) failed (%d):\n%s\n", res2,
		    gsasl_strerror (res2));
	      return;
	    }

	  if (debug)
	    printf ("C: %s [%c]\n", s1, ret_char (res2));
	}
      while (res1 != GSASL_OK || res2 != GSASL_OK);

      if (s1)
	{
	  gsasl_free (s1);
	  s1 = NULL;
	}

      if (debug)
	printf ("\n");

      gsasl_finish (client);
      gsasl_finish (server);
    }

  gsasl_done (ctx);
}
