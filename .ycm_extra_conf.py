def FlagsForFile( filename ):
    common = ['-I', '.', '-I', "../include/linmath.h"]
    if filename.rfind(".cpp")==len(filename)-4:
        return { 'flags': ['-x', 'c++']+common }
    else:
        return { 'flags': ['-x', 'c']+common }