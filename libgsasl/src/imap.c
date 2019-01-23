/* imap.c --- Implement IMAP profile of SASL login.
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

#include "imap.h"

int
imap_greeting (void)
{
  char *in;

  if (!readln (&in))
    return 0;

  return 1;
}

int
imap_has_starttls (void)
{
  char *in;
  int has_tls = 0;

  if (!writeln (". CAPABILITY"))
    return 0;

  if (!readln (&in))
    return 0;

  has_tls = strstr (in, "STARTTLS") != NULL;

  if (!readln (&in))
    return 0;

  return has_tls;
}

int
imap_starttls (void)
{
  char *in;

  if (!writeln (". STARTTLS"))
    return 0;

  if (!readln (&in))
    return 0;

  return 1;
}

int
imap_select_mechanism (char **mechlist)
{
  char *in;

  if (args_info.server_flag)
    {
      if (!args_info.quiet_given)
	fprintf (stderr, _("Chose SASL mechanisms:\n"));
      if (!readln (&in))
	return 0;
      *mechlist = in;
    }
  else
    {
      if (!writeln (". CAPABILITY"))
	return 0;

      if (!readln (&in))
	return 0;

      /* XXX parse IMAP capability line */

      *mechlist = in;

      if (!readln (&in))
	return 0;
    }

  return 1;
}

int
imap_authenticate (const char *mech)
{
  if (args_info.server_flag)
    {
      if (!args_info.quiet_given)
	fprintf (stderr, _("Using mechanism:\n"));
      puts (mech);
    }
  else
    {
      char *buf;
      int rc;
      int len;

      len = asprintf (&buf, ". AUTHENTICATE %s", mech);
      if (len < 0)
	return 0;
      rc = writeln (buf);
      free (buf);
      if (!rc)
	return 0;
    }

  return 1;
}

int
imap_step_send (const char *data)
{
  char *buf;
  int rc;
  int len;

  if (args_info.server_flag)
    len = asprintf (&buf, "+ %s", data);
  else
    len = asprintf (&buf, "%s", data);
  if (len < 0)
    return 0;
  rc = writeln (buf);
  free (buf);
  if (!rc)
    return 0;

  return 1;
}

/* Return 1 on token, 2 on protocol success, 3 on protocol fail, 0 on
   errors. */
int
imap_step_recv (char **data)
{
  char *p;

  if (!readln (data))
    return 0;

  p = *data;

  if (!args_info.server_flag)
    {
      /* skip untagged responses which can be returned by the server after
         authentication (e.g. dovecot returns new '* CAPABILITY' information
         before the final '. OK'). */
      while (*p == '*')
	{
	  if (!readln (data))
	    return 0;
	  p = *data;
	}

      if (strlen (p) >= 4 && strncmp (p, ". OK", 4) == 0)
	return 2;

      if (strlen (p) >= 2 && strncmp (p, ". ", 2) == 0)
	return 3;

      if (strlen (p) >= 2 && strncmp (p, "+ ", 2) == 0)
	memmove (&p[0], &p[2], strlen (p) - 1);
      /* This is a workaround for servers (e.g., Microsoft Exchange)
	 that return '+' instead of the correct '+ '.  */
      else if (strcmp (p, "+\n") == 0)
	p[0] = '\0';
      else
	{
	  fprintf (stderr, _("warning: server did not return a token\n"));
	  return 3;
	}
    }

  if (p[strlen (p) - 1] == '\n')
    p[strlen (p) - 1] = '\0';
  if (p[strlen (p) - 1] == '\r')
    p[strlen (p) - 1] = '\0';

  return 1;
}

int
imap_logout (void)
{
  char *in;

  if (!writeln (". LOGOUT"))
    return 0;

  /* read "* BYE ..." */
  if (!readln (&in))
    return 0;

  free (in);

  /* read ". OK ..." */
  if (!readln (&in))
    return 0;

  free (in);

  return 1;
}
