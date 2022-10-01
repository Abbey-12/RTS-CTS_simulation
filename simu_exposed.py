import sem
import numpy as np
import pandas as pd
import os
import matplotlib.pyplot as plt


def main():

    # Define campaign parameters
    ############################
    print("Start")
    script = 'exposed-terminal'
    cmp_name = 'sem_example' # define a campaign name in order to create different folders corresponding to a specific configuration
    ns_path = os.path.join(os.path.dirname(os.path.realpath(__file__)))
    campaign_dir = "./campaigns/"+cmp_name

    # Create campaign
    #################
    print("Campaign Started ..")
    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir,
        runner_type='ParallelRunner',
        check_repo=False,
        overwrite=False)

    # Parameter space
    #################

    datarate = [1, 2, 3,4]

    param_combinations = {
        'datarate': datarate
    }

    # Run simulations
    #################

    nruns = 10

    campaign.run_missing_simulations(sem.list_param_combinations(param_combinations),
        runs=nruns)

    print("Simulations done.")
    avg_retx1=[]
    avg_retx2=[]
  
    for rate in datarate:
        retx1=[]
        retx2=[]
        

        for result in campaign.db.get_results({'datarate': rate}):
            c1=0
            c2=0
            
            available_files = campaign.db.get_result_files (result)
            try:
                with open (available_files['wifi-st0-2-0.tr'], 'r') as f:
                    for line in f:
                        # print(line)
                        words1= line.split(',')
                        for word1 in words1:

                            # print(words)
                            if(word1==' Retry=1'): 
                                c1+=1
                with open (available_files['wifi-st1-3-0.tr'], 'r') as f:
                    for line in f:
                        # print(line)
                        words2= line.split(',')
                        for word2 in words2:

                            # print(words)
                            if(word2==' Retry=1'): 
                                c2+=1   
                                        
                retx1.append(c1)
                retx2.append(c2)
                

            except:
                print('Nothing to read.')
        avg_retx1.append(np.average(retx1))
        avg_retx2.append(np.average(retx2))
        

    print(f'\nAverage no. retx at sta1 with different datarate: {avg_retx1}')
    print(f'\nAverage no. retx at sta2 with different datarate: {avg_retx2}')
   
    _, (ax0, ax1) = plt.subplots (nrows=2)
    ax0.errorbar (datarate, avg_retx1)
    ax0.grid()
    ax0.set_xscale ('log')
    ax0.set_xlabel ('Datarate [Mbps]')
    ax0.set_ylabel ('Retx by sta1 ')
    
    ax1.errorbar (datarate, avg_retx2)
    ax1.grid()
    ax1.set_xscale ('log')
    ax1.set_xlabel ('Datarate [Mbps]')
    ax1.set_ylabel ('Retx by sta2')

    
    plt.show()
if __name__=='__main__':
    main()