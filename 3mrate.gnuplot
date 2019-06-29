
set grid
set term pngcairo size 1024,512


set datafile sep ','


set output '3mrate_rate.png'
plot \
        '3mrate.csv' u 3:($6*100) w l title "3m", \
        '3mrate.csv' u 3:($10*100) w l title 'OIS', \
        '3mrate.csv' u 3:($7==1?$6*100:1/0) w points title "3m-knots", \
        '3mrate.csv' u 3:($11==1?$10*100:1/0) w points title "OIS-knots"
set output '3mrate_df.png'
plot \
        '3mrate.csv' u 3:($4) w l title "3m", \
        '3mrate.csv' u 3:($8) w l title 'OIS', \
        '3mrate.csv' u 3:($7==1?$4:1/0) w points title "3m-knots", \
        '3mrate.csv' u 3:($11==1?$8:1/0) w points title "OIS-knots"

# vim:ft=gnuplot
