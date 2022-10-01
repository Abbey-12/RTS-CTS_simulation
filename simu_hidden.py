
    


import sem
import numpy as np
import pandas as pd
import os
import matplotlib.pyplot as plt


def main():

    # Define campaign parameters
    ############################
    print("Start")
    script = 'hidden_terminal'
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

    datarate = [1, 2, 3, 5]

    param_combinations = {
        'datarate': datarate
    }

    # Run simulations
    #################

    nruns = 10

    campaign.run_missing_simulations(sem.list_param_combinations(param_combinations),
        runs=nruns)

    print("Simulations done.")

    avg_txthr = []
    ce_txthr = []

    avg_rxthr = []
    ce_rxthr = []

    ave_loss=[]
    ce_loss=[]
    
    
    avg_retx1=[]
    avg_retx2=[]

    for rate in datarate:
        txthr_runs = []
        rxthr_runs = [] 
        loss_rate=[]
        retx1=[]
        retx2=[]

        for result in campaign.db.get_results({'datarate': rate}):
            c1=0
            c2=0
            available_files = campaign.db.get_result_files (result)
            try:
                
                rx_data = pd.read_csv (available_files['rx_trace.txt'], delimiter = "\t", skiprows=0, header=None)
                time = rx_data[0].iloc[-1] - rx_data[0].iloc[0]
                rxthr = np.sum(rx_data[1])*8/time/1e6

                tx_data = pd.read_csv (available_files['tx_trace.txt'], delimiter = "\t", skiprows=0, header=None)
                txthr = np.sum(tx_data[1])*8/time/1e6
                rx = (np.sum(tx_data[1]*8))/2/1000
                tx = np.sum(tx_data[1]*8)/1000
                loss = (tx-rx)*100/tx
                
                with open (available_files['wifi-st0-0-0.tr'], 'r') as f:
                    for line in f:
                        # print(line)
                        words1= line.split(',')
                        for word1 in words1:

                            # print(words)
                            if(word1==' Retry=1'): 
                                c1+=1
                with open (available_files['wifi-st1-0-0.tr'], 'r') as f:
                    for line in f:
                        # print(line)
                        words2= line.split(',')
                        for word2 in words2:

                            # print(words)
                            if(word2==' Retry=1'): 
                                c2+=1               
                retx1.append(c1)
                retx2.append(c2)
    

                txthr_runs.append(txthr)
                rxthr_runs.append(rxthr) 
                loss_rate.append(loss)
            except:
                print('Nothing to read.')

        
        avg_txthr.append (np.average (txthr_runs))
        std_txthr = np.std (txthr_runs)
        ce_txthr.append (1.96*std_txthr/len(txthr_runs))

        avg_rxthr.append (np.average (txthr_runs))
        std_rxthr = np.std (rxthr_runs)
        ce_rxthr.append (1.96*std_rxthr/len(rxthr_runs))

        ave_loss.append(np.average(loss_rate))
        std_loss=np.std(loss_rate)
        ce_loss.append (1.96*std_loss/len(loss_rate))
        avg_retx1.append(np.average(retx1))
        avg_retx2.append(np.average(retx2))
        

    print(f'\nAverage Txthr: {avg_txthr} \nCE: {ce_txthr}')
    print(f'\nAverage Rxthr: {avg_rxthr} \nCE: {ce_rxthr}')
    print(f'\nAverage loss_rate: {ave_loss} \nCE: {ce_loss}')
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
    plt.title('Number of retransmission with no Rts/Cts ')


if __name__=='__main__':
    main()
