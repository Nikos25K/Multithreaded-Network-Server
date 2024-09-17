cd ../bin/

#if no oarguments were given, set $1 to 18 and $2 to 7856
if [ $# -eq 0 ]; then
    set -- 18 7856
fi

#if only one argument was given, set $2 to 7856
if [ $# -eq 1 ]; then
    set -- $1 7856
fi

./jobCommander linux$1.di.uoa.gr $2 issueJob ./progDelay 10 &
./jobCommander linux$1.di.uoa.gr $2 issueJob ./progDelay 10 &
./jobCommander linux$1.di.uoa.gr $2 issueJob ./progDelay 10 &
./jobCommander linux$1.di.uoa.gr $2 issueJob ./progDelay 10 &
./jobCommander linux$1.di.uoa.gr $2 issueJob ./progDelay 10 &
./jobCommander linux$1.di.uoa.gr $2 issueJob ./progDelay 10 &
./jobCommander linux$1.di.uoa.gr $2 setConcurrency 4
sleep 1
./jobCommander linux$1.di.uoa.gr $2 poll
./jobCommander linux$1.di.uoa.gr $2 stop job_4
./jobCommander linux$1.di.uoa.gr $2 stop job_5
sleep 1
./jobCommander linux$1.di.uoa.gr $2 poll
./jobCommander linux$1.di.uoa.gr $2 exit