<?php

define('HTML_PREFIX','<!DOCTYPE html><html><head><meta http-equiv="Content-Type" content="text/html;charset=Windows-1250"/><link rel="stylesheet" type="text/css" href="gpc2html.css"/></head><body><table>');
define('HTML_SUFFIX','</table></body></html>');
define('HTML_ROW_ERROR','<tr><td class="error" colspan="7">{ERROR}</td></tr>');
define('HTML_ROW_074','
<tr class="row-074"><td class="label">Období:</td><td class="value value-fromtill" colspan="2">{DATEFROM} - {DATETILL}</td><td class="label" colspan="2">Počáteční zůstatek:</td><td class="value value-balancefrom" colspan="2">{BALANCEFROM}</td></tr>
<tr class="row-074"><td class="label">Účet:</td><td class="value value-accnumber" colspan="2">{ACCOUNTNUMBER}/0300</td><td class="label" colspan="2">Konečný zůstatek:</td><td class="value value-balancetill" colspan="2">{BALANCETILL}</td></tr>
<tr class="row-074"><td class="label">Název účtu:</td><td class="value value-accholder" colspan="2">{ACCOUNTHOLDER}</td><td class="label" colspan="2">Celkové výdaje:</td><td class="value value-sumdebit" colspan="2">{SUMDEBIT}</td></tr>
<tr class="row-074"><td class="label">Pořadové číslo výpisu:</td><td class="value value-stmtid" colspan="2">{STATEMENDID}</td><td class="label" colspan="2">Celkové příjmy:</td><td class="value value-sumcredit" colspan="2">{SUMCREDIT}</td></tr>
<tr class="row-075-1"><td class="label">Datum</td><td class="label">Označení platby</td><td class="label">Název protiúčtu</td><td class="label"></td><td class="label"></td><td class="label">Identifikace</td><td class="label">Částka</td></tr>
<tr class="row-075-2"><td class="label">Valuta</td><td class="label">Protiúčet nebo poznámka</td><td class="label">VS</td><td class="label">KS</td><td class="label">SS</td><td class="label"></td><td class="label"></td></tr>
');
define('HTML_ROW_075','
<tr class="row-075-1 altbg-{LINEMOD2} acctcode-{RAWACCOUNTINGCODE}"><td class="value value-dateacct">{DATEACCOUNTING}</td><td class="value value-acctcode">{ACCOUNTINGCODE}</td><td class="value value-accholder2">{ACCOUNTHOLDER2}</td><td class="value"></td><td class="value"></td><td class="value value-xactid">{TRANSACTIONID}</td><td class="value value-balance">{BALANCE}&nbsp;{CURRENCY}</td></tr>
<tr class="row-075-2 altbg-{LINEMOD2}"><td class="value value-dateconv">{DATECONV}</td><td class="value value-accnumber2">{ACCOUNTNUMBER2}</td><td class="value value-vs">{VSYMBOL}</td><td class="value value-ks">{KSYMBOL}</td><td class="value value-ss">{SSYMBOL}</td><td class="value"></td><td class="value"></td></tr>
');
define('HTML_ROW_076','
<tr class="row-076 altbg-{LINEMOD2}"><td class="value value-comment1" colspan="7">{COMMENT}</td></tr>
');
define('HTML_ROW_078','
<tr class="row-078 altbg-{LINEMOD2}"><td class="value value-msg1" colspan="2">{MSG1}</td><td class="value value-msg2" colspan="3">{MSG2}</td><td class="value value-msg3" colspan="2">{MSG3}</td></tr>
');
define('HTML_ROW_079','
<tr class="row-079 altbg-{LINEMOD2}"><td class="value value-msg4" colspan="2">{MSG4}</td><td class="value value-msg5" colspan="3">{MSG5}</td><td class="value value-msg6" colspan="2">{MSG6}</td></tr>
');

function LoadTemplate($sTemplate,$aVars = array())
{
    foreach($aVars as $sKey=>$sValue)
    {
        $sTemplate = str_replace('{'.$sKey.'}',htmlspecialchars($sValue),$sTemplate);
    }

    return $sTemplate;
}

function FormatMoneyFast($sInt,$sFrac)
{
    $sText = intval(ltrim($sInt,'0')).','.$sFrac;

    $sOut = substr($sText,-6);
    $sText = substr($sText,0,-6);

    while($sText!=='')
    {
        $sOut = substr($sText,-3).'.'.$sOut;
        $sText = substr($sText,0,-3);
    }

    return $sOut;
}

function AccountingCodeToText($nCode)
{
    switch($nCode)
    {
    case 1: return 'Odchozí platba';
    case 2: return 'Příchozí platba';
    case 4: return 'Storno odchozí platby';
    case 5: return 'Storno příchozí platby';
    }

    return 'Neznámé zaúčtování';
}

function CurrencyCodeToText($nCode)
{
    switch($nCode)
    {
    case 36:  return 'AUD';
    case 124: return 'CAD';
    case 156: return 'CNY';
    case 203: return 'CZK';
    case 208: return 'DKK';
    case 978: return 'EUR';
    case 826: return 'GBP';
    case 191: return 'HRK';
    case 348: return 'HUF';
    case 756: return 'CHF';
    case 392: return 'JPY';
    case 578: return 'NOK';
    case 985: return 'PLN';
    case 946: return 'RON';
    case 643: return 'RUB';
    case 752: return 'SEK';
    case 949: return 'TRY';
    case 840: return 'USD';
    }

    return '???';
}

function GpcToHtmlLine($nLine,$sLine,&$nTXNLine)
{
    if(substr($sLine,-2)==="\r\n")
    {
        $sLine = substr($sLine,0,-2);

        if(preg_match('/^074(\\d{16})(.{20})(\\d{2})(\\d{2})(\\d{2})(\\d{12})(\\d{2})([+\\-])(\\d{12})(\\d{2})([+\\-])(\\d{12})(\\d{2})0(\\d{12})(\\d{2})0(\\d{3})(\\d{2})(\\d{2})(\\d{2}) {14}$/',$sLine,$aMatches))
        {// header 074
            return LoadTemplate(HTML_ROW_074,array
            (
                'ACCOUNTNUMBER'=>ltrim($aMatches[1],'0'),
                'ACCOUNTHOLDER'=>rtrim($aMatches[2],' '),
                'DATEFROM'=>$aMatches[3].'.'.$aMatches[4].'.'.$aMatches[5],
                'BALANCEFROM'=>$aMatches[8].FormatMoneyFast($aMatches[6],$aMatches[7]),
                'BALANCETILL'=>$aMatches[11].FormatMoneyFast($aMatches[9],$aMatches[10]),
                'SUMDEBIT'=>FormatMoneyFast($aMatches[12],$aMatches[13]),
                'SUMCREDIT'=>FormatMoneyFast($aMatches[14],$aMatches[15]),
                'STATEMENDID'=>ltrim($aMatches[16],'0'),
                'DATETILL'=>$aMatches[17].'.'.$aMatches[18].'.'.$aMatches[19],
            ));
        }
        elseif(preg_match('/^075(\\d{16})(\\d{16})(\\d{13})(\\d{10})(\\d{2})(\\d)(\\d{10})00(\\d{4})(\\d{4})(\\d{10})(\\d{2})(\\d{2})(\\d{2})(.{20})(\\d{5})(\\d{2})(\\d{2})(\\d{2})$/',$sLine,$aMatches))
        {// detail 075
            $nTXNLine++;

            $sAccNumber2 = ltrim($aMatches[2],'0');
            $sBankCode2  = $aMatches[8];

            if($sAccNumber2!=='')
            {
                $sAccNumber2.= '/'.$sBankCode2;
            }

            return LoadTemplate(HTML_ROW_075,array
            (
                'LINEMOD2'=>$nTXNLine%2,
                'ACCOUNTNUMBER'=>ltrim($aMatches[1],'0'),
                'ACCOUNTNUMBER2'=>$sAccNumber2,
                'TRANSACTIONID'=>$aMatches[3],
                'BALANCE'=>FormatMoneyFast($aMatches[4],$aMatches[5]),
                'RAWACCOUNTINGCODE'=>$aMatches[6],
                'ACCOUNTINGCODE'=>AccountingCodeToText($aMatches[6]),
                'VSYMBOL'=>ltrim($aMatches[7],'0'),
                'KSYMBOL'=>ltrim($aMatches[9],'0'),
                'SSYMBOL'=>ltrim($aMatches[10],'0'),
                'DATECONV'=>$aMatches[11].'.'.$aMatches[12].'.'.$aMatches[13],
                'ACCOUNTHOLDER2'=>$aMatches[14],
                'CURRENCY'=>CurrencyCodeToText(ltrim($aMatches[15],'0')),
                'DATEACCOUNTING'=>$aMatches[16].'.'.$aMatches[17].'.'.$aMatches[18],
            ));
        }
        elseif(preg_match('/^076(.{26})(\\d{2})(\\d{2})(\\d{2})(.{93})$/',$sLine,$aMatches))
        {// more info 076
            $sComment = rtrim($aMatches[5],' ');

            if($sComment==='')
            {
                return '';
            }

            return LoadTemplate(HTML_ROW_076,array
            (
                'LINEMOD2'=>$nTXNLine%2,
                'BANKTRANSACTIONID'=>$aMatches[1],
                'DATEDRAW'=>$aMatches[2].'.'.$aMatches[3].'.'.$aMatches[4],
                'COMMENT'=>$sComment,
            ));
        }
        elseif(preg_match('/^078(.{35})(.{35})(.{55})$/',$sLine,$aMatches))
        {// message 078
            $sMsg1 = rtrim($aMatches[1],' ');
            $sMsg2 = rtrim($aMatches[2],' ');
            $sMsg3 = rtrim($aMatches[3],' ');

            if($sMsg1==='' && $sMsg2==='' && $sMsg3==='')
            {
                return '';
            }

            return LoadTemplate(HTML_ROW_078,array
            (
                'LINEMOD2'=>$nTXNLine%2,
                'MSG1'=>$sMsg1,
                'MSG2'=>$sMsg2,
                'MSG3'=>$sMsg3,
            ));
        }
        elseif(preg_match('/^079(.{35})(.{35})(.{35}).{20}$/',$sLine,$aMatches))
        {// message 079
            $sMsg4 = rtrim($aMatches[1],' ');
            $sMsg5 = rtrim($aMatches[2],' ');
            $sMsg6 = rtrim($aMatches[3],' ');

            if($sMsg4==='' && $sMsg5==='' && $sMsg6==='')
            {
                return '';
            }

            return LoadTemplate(HTML_ROW_079,array
            (
                'LINEMOD2'=>$nTXNLine%2,
                'MSG4'=>$sMsg4,
                'MSG5'=>$sMsg5,
                'MSG6'=>$sMsg6,
            ));
        }
    }

    return LoadTemplate(HTML_ROW_ERROR,array('ERROR'=>'Chyba v řádku '.$nLine));
}

function GpcToHtml($sGpcFile,$sHtmlFile)
{
    $aInput = file($sGpcFile);
    $aOutput = array();
    $nTXNLine = 0;

    foreach($aInput as $nIdx=>$sLine)
    {
        $aOutput[] = GpcToHtmlLine($nIdx+1,$sLine,$nTXNLine);
    }

    if(($hFile = fopen($sHtmlFile,'wb'))!==false)
    {
        fwrite($hFile,HTML_PREFIX);
        fwrite($hFile,implode('',$aOutput));
        fwrite($hFile,HTML_SUFFIX);
        fclose($hFile);
    }
}

function Main()
{
    foreach(glob('*.GPC') as $sFile)
    {
        GpcToHtml($sFile,substr($sFile,0,-3).'htm');
    }
}

Main();
