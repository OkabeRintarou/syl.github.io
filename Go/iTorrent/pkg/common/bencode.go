package common

import (
    "errors"
    "sort"
    "strconv"
    "strings"
)

func Encode(data interface{}) (string, error) {
    switch data.(type) {
    case int:
        return EncodeInt(data.(int))
    case string:
        return EncodeString(data.(string))
    case []interface{}:
        return EncodeList(data.([]interface{}))
    case map[string]interface{}:
        return EncodeMap(data.(map[string]interface{}))
    default:
        return "", errors.New("non encoded data")
    }
}

func EncodeInt(data int) (string, error) {
    return strings.Join([]string{"i", strconv.Itoa(data), "e"}, ""), nil
}

func EncodeString(data string) (string, error) {
    return strings.Join([]string{strconv.Itoa(len(data)), ":", data}, ""), nil
}

func EncodeList(datas []interface{}) (string, error) {
    ret := make([]string, 0, len(datas)+2)

    ret = append(ret, "l")
    for _, data := range datas {
        elem, err := Encode(data)
        if err != nil {
            return "", err
        }
        ret = append(ret, elem)
    }
    ret = append(ret, "e")
    return strings.Join(ret, ""), nil
}

func EncodeMap(data map[string]interface{}) (string, error) {
    ret := make([]string, len(data)+2)

    ret = append(ret, "d")

    keys := make([]string, 0, len(data))
    for k := range data {
        keys = append(keys, k)
    }
    sort.Strings(keys)
    for _, k := range keys {
        elem, _ := EncodeString(k)
        ret = append(ret, elem)
        elem, err := Encode(data[k])
        if err != nil {
            return "", err
        }
        ret = append(ret, elem)
    }

    ret = append(ret, "e")
    return strings.Join(ret, ""), nil
}

var ErrBadFormat = errors.New("bad format")

func Decode(data string) (r interface{}, err error) {
    r, _, err = decodeItem(data)
    return
}

func decodeItem(data string) (interface{}, int, error) {
    if len(data) == 0 {
        return nil, 0, ErrBadFormat
    }
    switch data[0] {
    case 'i':
        return decodeInt(data)
    case 'l':
        return decodeList(data)
    case 'd':
        return decodeMap(data)
    default:
        return decodeString(data)
    }
    return nil, 0, nil
}

func decodeInt(data string) (num int, eat int, err error) {
    if len(data) < 3 || data[0] != 'i' {
        return 0, 0, ErrBadFormat
    }
    for eat = 1; eat < len(data); eat++ {
        if data[eat] == 'e' {
            break
        }
    }
    if eat == len(data) {
        return 0, eat, ErrBadFormat
    }

    num, err = strconv.Atoi(data[1:eat])
    eat++
    return
}

func decodeString(data string) (str string, eat int, err error) {
    eat = strings.IndexByte(data, ':')
    if eat == -1 {
        return "", eat, ErrBadFormat
    }
    var len int
    len, err = strconv.Atoi(data[:eat])
    if err != nil {
        return "", eat, ErrBadFormat
    }

    return data[eat+1 : eat+len+1], eat + len + 1, nil
}

func decodeList(data string) (list []interface{}, eat int, err error) {
    if len(data) < 1 || data[0] != 'l' {
        return nil, 0, ErrBadFormat
    }
    list = make([]interface{}, 0, 4)
    eat++

    for eat < len(data) && data[eat] != 'e' {
        var item interface{}
        var advance int
        item, advance, err = decodeItem(data[eat:])
        if err != nil {
            return
        }

        list = append(list, item)
        eat += advance
    }
    if eat == len(data) {
        err = ErrBadFormat
    } else {
        eat++
    }
    return
}

func decodeMap(data string) (mp map[string]interface{}, eat int, err error) {
    if len(data) < 1 || data[0] != 'd' {
        return nil, 0, ErrBadFormat
    }
    mp = make(map[string]interface{})
    eat++

    for eat < len(data) && data[eat] != 'e' {
        var key string
        var value interface{}
        var advance int
        key, advance, err = decodeString(data[eat:])
        if err != nil {
            return
        }
        eat += advance
        value, advance, err = decodeItem(data[eat:])
        if err != nil {
            return
        }
        eat += advance
        mp[key] = value
    }

    if eat == len(data) {
        err = ErrBadFormat
    } else {
        eat++
    }
    return
}

func BencodeGet(data interface{}, path string) (interface{}, error) {
    paths := strings.Split(path, "/")
    if len(paths) == 0 {
        return "", errors.New("invalid path")
    }

    keyNotExist := errors.New("key not exist")
    cur := data

    for _, k := range paths {
        m, ok := cur.(map[string]interface{})
        if !ok {
            return "", keyNotExist
        }
        if cur, ok = m[k]; !ok {
            return "", keyNotExist
        }
    }
    return cur, nil
}

func BencodeGetString(data interface{}, path string) (string, error) {

    d, err := BencodeGet(data, path)
    if err != nil {
        return "", err
    }
    s, ok := d.(string)
    if !ok {
        return "", errors.New("not string")
    }
    return s, nil
}

func BencodeGetInt(data interface{}, path string) (int, error) {
    d, err := BencodeGet(data, path)
    if err != nil {
        return 0, err
    }
    s, ok := d.(int)
    if !ok {
        return 0, errors.New("not int")
    }
    return s, nil
}
