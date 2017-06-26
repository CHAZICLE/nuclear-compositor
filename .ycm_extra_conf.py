import git

def FlagsForFile( filename ):
    git_repo = git.Repo(".", search_parent_directories=True)
    git_root = git_repo.git.rev_parse("--show-toplevel")
    common = ['-Wall', '-DNUCLEAR_OSVR=1', '-I', git_root+"/src", '-I', git_root+"/include/linmath.h"]
    print(common)
    if filename.rfind(".cpp")==len(filename)-4:
        return { 'flags': ['-x', 'c++']+common }
    else:
        return { 'flags': ['-x', 'c']+common }
