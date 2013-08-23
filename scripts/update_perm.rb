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
  system("find . -name '*.sh' | xargs git update-index --chmod=+x ")
  system("find . -name '*.sh' | xargs git update-index --chmod=+x ")
  system("cd sdk && git commit -a -m 'Update script permission' && git push origin master && cd ..")
  system("cd apps && git commit -a -m 'Update script permission' && git push origin master && cd ..")
  system("git commit -a -m 'Update shell script permissions.'")
  system("git push origin #{branch}")
end
