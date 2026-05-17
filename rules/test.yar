rule EICAR_Test_String
{
    meta:
        description = "EICAR Standard Antivirus Test File"
        author = "MiniEDR"

    strings:
        $eicar = "X5O!P%@AP[4\\PZX54(P^)7CC)7}$EICAR-STANDARD-ANTIVIRUS-TEST-FILE!$H+H*"

    condition:
        $eicar
}