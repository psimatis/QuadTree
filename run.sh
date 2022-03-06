#cmake -G "Unix Makefiles" .
#cmake -S . -B build
#cmake --build build -j --clean-first
# cmake --install build
Q=(Mix Rng Knn)
T=(S T ST)
W=(W E R)
for q in ${Q[@]}; do
    for t in ${T[@]}; do
        for w in ${W[@]}; do
            QF="${q}/${t}-L1e7-I1e6-${w}H"
            echo "Experiments/${QF}"
            mkdir -p "Experiments/${QF}"
            ./Index $PWD $QF 128 128
            # lldb -- Index $PWD $QF 128 128
        done
    done
done
