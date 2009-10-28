function preview()
    local file = props['FilePath']
    print(file)
    os.execute("\"C:/Program Files/Internet Explorer/iexplore.exe\" " .. file)
end
