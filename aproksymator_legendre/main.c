#include "points.h"
#include "coefficients.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

char *usage =
    "Usage: %s -s coefficients-file [-p points-file] [ -g gnuplot-file [-f from_x -t to_x -n n_points ] ]\n"
    "            if points-file is given then\n"
    "               reads discrete 2D points from points-file\n"
    "               writes function coefficients to coefficients-file\n"
    "               - number of points should be >= 4\n"
    "            else (points-file not given)\n"
    "               reads coefficients from coefficients-file\n"
    "            endif\n"
    "            if gnuplot-file is given then\n"
    "               makes table of n_points within <from_x,to_x> range\n"
    "               - from_x defaults to x-coordinate of the first point in points-file,\n"
    "               - to_x defaults to x-coordinate of the last point\n"
    "               - n_points defaults to 100\n"
    "               - n_points must be > 1\n"
    "            endif\n";

int main (int argc, char **argv)
{
    int opt;
    char *inp = NULL;
    char *out = NULL;
    char *gpt = NULL;
    double fromX = 0;
    double toX = 0;
    int n = 100;
    char *progname= argv[0];

    points_t pts;
    coefficients_t cof;

    pts.n = 0;
    cof.base = 0;

    /* process options, save user choices */
    while ((opt = getopt (argc, argv, "p:s:g:f:t:n:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            inp = optarg;
            break;
        case 's':
            out = optarg;
            break;
        case 'g':
            gpt = optarg;
            break;
        case 'f':
            fromX = atof (optarg);
            break;
        case 't':
            toX = atof (optarg);
            break;
        case 'n':
            n = atoi (optarg);
            break;
        default:                   /* '?' */
            fprintf (stderr, usage, progname);
            exit (EXIT_FAILURE);
        }
    }
    if( optind < argc )
    {
        fprintf( stderr, "\nBad parameters!\n" );
        for( ; optind < argc; optind++ )
            fprintf( stderr, "\t\"%s\"\n", argv[optind] );
        fprintf( stderr, "\n" );
        fprintf( stderr, usage, progname );
        exit( EXIT_FAILURE );
    }

    /* if points-file was given, then read points, generate spline, save it to file */
    if (inp != NULL)
    {
        FILE *ouf = NULL; /* we shall open it later, when we shall get points */

        FILE *inf = fopen (inp, "r");
        if (inf == NULL)
        {
            fprintf (stderr, "%s: can not read points file: %s\n\n", argv[0], inp);
            exit (EXIT_FAILURE);
        }

        if (read_pts_failed (inf, &pts))
        {
            fprintf (stderr, "%s: bad contents of points file: %s\n\n", argv[0], inp);
            exit (EXIT_FAILURE);
        }
        else
            fclose (inf);

        ouf = fopen (out, "w");
        if (ouf == NULL)
        {
            fprintf (stderr, "%s: can not write coefficients file: %s\n\n", argv[0], out);
            exit (EXIT_FAILURE);
        }

        make_cof (&pts, &cof);

        if( cof.base > 0 )
            write_cof (&cof, ouf);

        fclose (ouf);
    }
    else if (out != NULL)   /* if point-file was NOT given, try to read coefficients from a file */
    {
        FILE *coff = fopen (out, "r");
        if (coff == NULL)
        {
            fprintf (stderr, "%s: can not read coefficients file: %s\n\n", argv[0], inp);
            exit (EXIT_FAILURE);
        }
        if (read_cof (coff, &cof))
        {
            fprintf (stderr, "%s: bad contents of coefficients file: %s\n\n", argv[0], inp);
            exit (EXIT_FAILURE);
        }
    }
    else     /* ponts were not given nor coefficients was given -> it is an error */
    {
        fprintf (stderr, usage, argv[0]);
        exit (EXIT_FAILURE);
    }

    if (cof.base < 1)   /* check if there are a valid coefficients */
    {
        fprintf (stderr, "%s: bad coefficients: base=%d\n\n", argv[0], cof.base - 1);
        exit (EXIT_FAILURE);
    }

    /* check if plot was requested and generate it if yes */
    if (gpt != NULL && n > 1)
    {
        FILE *gpf = fopen (gpt, "w");
        int i;
        double dx;
        if( fromX == 0 && toX == 0 )   /* calculate plot range if it was not specified */
        {
            if( pts.n > 1 )
            {
                fromX= pts.x[0];
                toX=   pts.x[pts.n-1];
            }
            else
            {
                fromX= 0;
                toX= 1;
            }
        }
        dx = (toX - fromX) / (n - 1);

        if (gpf == NULL)
        {
            fprintf (stderr, "%s: can not write gnuplot file: %s\n\n", argv[0], gpt);
            exit (EXIT_FAILURE);
        }

        for (i = 0; i < n; i++)
            fprintf (gpf, "%g %g\n", fromX + i * dx, value_cof (&cof, fromX + i * dx));

        fclose (gpf);
    }

    return 0;
}
