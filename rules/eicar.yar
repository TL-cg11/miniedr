rule EICAR_Test_File
{
    meta:
        author = "SDY"
        description = "EICAR 표준 안티바이러스 테스트 파일 탐지"
        date = "2026-05-12"
        reference = "https://www.eicar.org/?page_id=3950"
        severity = "low"

    strings:
        $eicar_signature = "EICAR-STANDARD-ANTIVIRUS-TEST-FILE"
        $eicar_start = { 58 35 4F 21 50 25 40 41 50 }

    condition:
        $eicar_start at 0 and $eicar_signature
}