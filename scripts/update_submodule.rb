branches = ['master',
            'mac_center',
            'mac_center-simplified',
            'icell',
            'songri',
            'wolder',
            'bebook-cross-simplified',
            'simplicissimus']

branches.each do |branch|
        system("git checkout #{branch}")
        system("cd sdk && git checkout master && git pull origin master && cd ..")
        system("cd apps && git checkout master && git pull origin master && cd ..")
        system("git commit -a -m 'Update submodule refs.'")
end

system("git checkout ozon")
system("cd sdk && git checkout ozon && git pull origin ozon && cd ..")
system("cd apps && git checkout ozon && git pull origin ozon && cd ..")

system("git checkout experimental")
system("cd sdk && git checkout experimental && git pull origin experimental && cd ..")
system("cd apps && git checkout master && git pull origin master && cd ..")
